/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a hierarchical registry abstraction.  See uniconf.h.
 */
#include "uniconf.h"
#include "uniconfroot.h"
#include "uniconfgen.h"
#include "wvstream.h"
#include <assert.h>


UniConfKey UniConf::fullkey(const UniConfKey &k) const
{
    int n = k.numsegments();
    
    // this function is undefined if k isn't an ancestor!
    assert(k == xfullkey.first(n));
    
    return xfullkey.removefirst(n);
}


bool UniConf::exists() const
{
    return xroot->_exists(xfullkey);
}


bool UniConf::haschildren() const
{
    return xroot->_haschildren(xfullkey);
}


WvString UniConf::get(WvStringParm defvalue) const
{
    WvString value = xroot->_get(xfullkey);
    if (value.isnull())
        return defvalue;
    return value;
}


int UniConf::getint(int defvalue) const
{
    // also recognize bool strings as integers
    const char *strs[] = {
        "true", "yes", "on", "enabled",
        "false", "no", "off", "disabled"
    };
    const size_t numtruestrs = 4;

    WvString value = get();
    if (!value.isnull())
    {
        // try to recognize an integer
        char *end;
        int num = strtol(value.cstr(), &end, 0);
        if (end != value.cstr())
            return num; // was a valid integer
        
        // try to recognize a special string
        for (size_t i = 0; i < sizeof(strs) / sizeof(const char*); ++i)
            if (strcasecmp(value, strs[i]) == 0)
                return i < numtruestrs;
    }
    return defvalue;
}


bool UniConf::set(WvStringParm value) const
{
    return xroot->_set(xfullkey, value);
}


bool UniConf::setint(int value) const
{
    return xroot->_set(xfullkey, WvString(value));
}


bool UniConf::zap() const
{
    return xroot->_zap(xfullkey);
}


bool UniConf::refresh(UniConfDepth::Type depth) const
{
    return xroot->_refresh(xfullkey, depth);
}


bool UniConf::commit(UniConfDepth::Type depth) const
{
    return xroot->_commit(xfullkey, depth);
}


UniConfGen *UniConf::mount(WvStringParm moniker, bool refresh) const
{
    return xroot->_mount(xfullkey, moniker, refresh);
}


UniConfGen *UniConf::mountgen(UniConfGen *gen, bool refresh) const
{
    return xroot->_mountgen(xfullkey, gen, refresh);
}


void UniConf::unmount(UniConfGen *gen, bool commit) const
{
    return xroot->_unmount(xfullkey, gen, commit);
}


bool UniConf::ismountpoint() const
{
    return xroot->_ismountpoint(xfullkey);
}


UniConfGen *UniConf::whichmount(UniConfKey *mountpoint) const
{
    return xroot->_whichmount(xfullkey, mountpoint);
}


void UniConf::addwatch(UniConfDepth::Type depth, UniConfWatch *watch) const
{
    return xroot->_addwatch(xfullkey, depth, watch);
}


void UniConf::delwatch(UniConfDepth::Type depth, UniConfWatch *watch) const
{
    return xroot->_delwatch(xfullkey, depth, watch);
}


void UniConf::dump(WvStream &stream, bool everything) const
{
    UniConf::RecursiveIter it(*this);
    for (it.rewind(); it.next(); )
    {
        WvString value(it->get());
        if (everything || !!value)
            stream.print("%s = %s\n", it->fullkey(), value);
    }
}



/***** UniConf::Iter *****/

UniConf::Iter::Iter(const UniConf &root) 
    : IterBase(root),
    it(*root.rootobj(), root.fullkey())
{
}


bool UniConf::Iter::next()
{
    if (it.next())
    {
        current = top[it.key()];
        return true;
    }
    return false;
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(const UniConf &root)
    : IterBase(root)
{
}


void UniConf::RecursiveIter::rewind()
{
    itlist.zap();
    UniConf::Iter *subi = new UniConf::Iter(top);
    subi->rewind();
    itlist.prepend(subi, true);
}


bool UniConf::RecursiveIter::next()
{
    assert(!itlist.isempty()); // trying to seek past the end is illegal!
    
    UniConf::IterList::Iter i(itlist);
    for (i.rewind(); i.next(); )
    {
        if (i->next()) // NOTE: not the same as i.next()
        {
            // return the item first
            current = **i;
            
            // set up so next time, we go into its subtree
            if (current.haschildren())
            {
                UniConf::Iter *subi = new UniConf::Iter(current);
                subi->rewind();
                itlist.prepend(subi, true);
            }
            
            return true;
        }
        
        // otherwise, this iterator is empty; move up the tree
        i.xunlink();
    }
    
    // all done!
    return false;
}



/***** UniConf::XIter *****/

UniConf::XIter::XIter(const UniConf &_top, const UniConfKey &pattern)
    : IterBase(_top), pathead(pattern.first()),
    pattail(pattern.removefirst()), subit(NULL), it(NULL), recit(NULL)
{
    if (! pathead.iswild())
    {
        // optimization to collect as many consecutive non-wildcard
        // segments as possible in one go
        while (! pattail.isempty())
        {
            UniConfKey patnext(pattail.first());
            if (patnext.iswild())
                break;
            pathead.append(patnext);
            pattail = pattail.removefirst();
        }
    }
}


UniConf::XIter::~XIter()
{
    cleanup();
}


void UniConf::XIter::cleanup()
{
    if (subit)
    {
        delete subit;
        subit = NULL;
    }
    if (it)
    {
        delete it;
        it = NULL;
    }
    if (recit)
    {
        delete recit;
        recit = NULL;
    }
}


void UniConf::XIter::rewind()
{
    cleanup();
    ready = false;

    if (pathead.isempty())
    {
        current = top;
        ready = current.exists();
    }
    else if (pathead == UniConfKey::RECURSIVE_ANY)
    {
        recit = new UniConf::RecursiveIter(top);
        recit->rewind();
        if (UniConfKey::EMPTY.matches(pattail))
        {
            // pattern includes self
            current = top;
            ready = current.exists();
        }
    }
    else if (pathead == UniConfKey::ANY)
    {
        it = new UniConf::Iter(top);
        it->rewind();
    }
    else
    {
        // non-wildcard segment
        current = top[pathead];
        if (pattail.isempty())
        {
            // don't bother recursing if there are no deeper wildcard
            // elements (works together with optimization in constructor)
            ready = current.exists();
        }
        else
        {
            // more wildcards, setup recursion
            enter(current);
        }
    }
}


inline bool UniConf::XIter::qnext()
{
    if (subit) // currently in a sub-iterator
    {
        bool found = subit->next();
        if (found)
        {
            current = **subit;
            return true;
        }
        else
        {
            // end of this sub-iterator
            delete subit;
            subit = NULL;
            return false;
        }
    }
    else // no sub-iterator at all
        return false;
}


void UniConf::XIter::enter(const UniConf &child)
{
    subit = new UniConf::XIter(child, pattail);
    subit->rewind();
}


bool UniConf::XIter::next()
{
    if (ready)
    {
        ready = false;
        return true;
    }
    while (!qnext())
    {
        // UniConfKey::ANY
        if (it && it->next())
        {
            /* Not needed for now since we don't match partial keys
            if (! pathead.matches(it->key()))
                break;
            */
            enter(**it);
            continue;
        }
        // UniConfKey::RECURSIVE_ANY
        if (recit && recit->next())
        {
            enter(**recit);
            continue;
        }
        // anything else or finished
        return false;
    }
    
    // if we get here, qnext() returned true
    return true; 
}



/***** UniConf::SortedIterBase *****/

UniConf::SortedIterBase::SortedIterBase(const UniConf &root,
    UniConf::SortedIterBase::Comparator comparator) 
    : IterBase(root), xcomparator(comparator), xkeys(true)
{
}


UniConf::SortedIterBase::~SortedIterBase()
{
    _purge();
}


int UniConf::SortedIterBase::defcomparator(const UniConf &a,
    const UniConf &b)
{
    return a.fullkey().compareto(b.fullkey());
}


UniConf::SortedIterBase::Comparator
    UniConf::SortedIterBase::innercomparator = NULL;

int UniConf::SortedIterBase::wrapcomparator(const UniConf **a,
    const UniConf **b)
{
    return innercomparator(**a, **b);
}


void UniConf::SortedIterBase::_purge()
{
    count = xkeys.count();
    xkeys.zap();
}


void UniConf::SortedIterBase::_rewind()
{
    index = 0;
    count = xkeys.count();
    
    // This code is NOT reentrant because qsort makes it too hard
    innercomparator = xcomparator;
    qsort(xkeys.ptr(), count, sizeof(UniConf*),
        (int (*)(const void *, const void *))wrapcomparator);
}


bool UniConf::SortedIterBase::next()
{
    if (index >= count)
        return false;
    current = *xkeys[index];
    index += 1;
    return true;
}
