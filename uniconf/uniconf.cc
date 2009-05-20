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
#include <climits>
#include <algorithm>
#include <assert.h>


UniConf::UniConf(UniConfRoot *root, const UniConfKey &fullkey)
    : xroot(root), xfullkey(fullkey)
{ 
    // nothing special
}
    

UniConf::UniConf() : xroot(NULL), xfullkey(UniConfKey::EMPTY)
{ 
    // nothing special
}


UniConf::UniConf(const UniConf &other)
    : xroot(other.xroot), xfullkey(other.xfullkey)
{ 
    // nothing special
}


UniConf::~UniConf()
{ 
    // nothing special
}




UniConfKey UniConf::fullkey(const UniConfKey &k) const
{
    return k.subkey(xfullkey);
}


bool UniConf::exists() const
{
    return xroot->mounts.exists(xfullkey);
}


bool UniConf::haschildren() const
{
    return xroot->mounts.haschildren(xfullkey);
}


void UniConf::prefetch(bool recursive) const
{
    xroot->mounts.prefetch(xfullkey, recursive);
}


WvString UniConf::getme(WvStringParm defvalue) const
{
    WvString value = xroot->mounts.get(xfullkey);
    if (value.isnull())
        return defvalue;
    return value;
}


int UniConf::getmeint(int defvalue) const
{
    return xroot->mounts.str2int(getme(), defvalue);
}


void UniConf::setme(WvStringParm value) const
{
    xroot->mounts.set(xfullkey, value);
}


void UniConf::setmeint(int value) const
{
    setme(WvString(value));
}


void UniConf::move(const UniConf &dst) const
{
    dst.remove();
    copy(dst, true); 
    remove();
}


void UniConf::copy(const UniConf &dst, bool force) const
{
    // do the main key first
    dst.setme(getme());

    // now all the children
    RecursiveIter i(*this);
    for (i.rewind(); i.next(); )
    {
	UniConf dst2 = dst[i->fullkey(*this)];
	if (force || dst2.getme().isnull())
	    dst2.setme(i->getme());
    }
}


bool UniConf::refresh() const
{
    return xroot->mounts.refresh();
}


void UniConf::commit() const
{
    xroot->mounts.commit();
}


IUniConfGen *UniConf::mount(WvStringParm moniker, bool refresh) const
{
    return xroot->mounts.mount(xfullkey, moniker, refresh);
}


IUniConfGen *UniConf::mountgen(IUniConfGen *gen, bool refresh) const
{
    return xroot->mounts.mountgen(xfullkey, gen, refresh);
}


void UniConf::unmount(IUniConfGen *gen, bool commit) const
{
    return xroot->mounts.unmount(gen, commit);
}


bool UniConf::ismountpoint() const
{
    return xroot->mounts.ismountpoint(xfullkey);
}


IUniConfGen *UniConf::whichmount(UniConfKey *mountpoint) const
{
    return xroot->mounts.whichmount(xfullkey, mountpoint);
}


bool UniConf::isok() const
{
    IUniConfGen *gen = whichmount();
    return gen && gen->isok();
}


void UniConf::add_callback(void *cookie, const UniConfCallback &callback,
                           bool recurse) const
{
    xroot->add_callback(cookie, xfullkey, callback, recurse);
}


void UniConf::del_callback(void *cookie, bool recurse) const
{
    xroot->del_callback(cookie, xfullkey, recurse);
}


void UniConf::add_setbool(bool *flag, bool recurse) const
{
    xroot->add_setbool(xfullkey, flag, recurse);
}


void UniConf::del_setbool(bool *flag, bool recurse) const
{
    xroot->del_setbool(xfullkey, flag, recurse);
}


void UniConf::hold_delta()
{
    xroot->mounts.hold_delta();
}


void UniConf::unhold_delta()
{
    xroot->mounts.unhold_delta();
}


void UniConf::clear_delta()
{
    xroot->mounts.clear_delta();
}


void UniConf::flush_delta()
{
    xroot->mounts.flush_delta();
}


void UniConf::dump(WvStream &stream, bool everything) const
{
    UniConf::RecursiveIter it(*this);
    for (it.rewind(); it.next(); )
    {
        WvString value(it->getme());
        if (everything || !!value)
            stream.print("%s = %s\n", it->fullkey(), value);
    }
}



/***** UniConf::Iter *****/

UniConf::Iter::Iter(const UniConf &_top)
    : IterBase(_top)
{
    it = _top.rootobj()->mounts.iterator(top.fullkey());
    if (!it) it = new UniConfGen::NullIter;
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(const UniConf &_top)
    : IterBase(_top)
{
    it = _top.rootobj()->mounts.recursiveiterator(top.fullkey());
    if (!it) it = new UniConfGen::NullIter;
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
    : IterBase(root), xcomparator(comparator), xkeys()
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


static UniConf::SortedIterBase::Comparator innercomparator = NULL;

static bool wrapcomparator(const UniConf &a, const UniConf &b)
{
    return innercomparator(a, b) < 0;
}


void UniConf::SortedIterBase::_purge()
{
    count = xkeys.size();
    xkeys.clear();
}


void UniConf::SortedIterBase::_rewind()
{
    index = 0;
    count = xkeys.size();
    
    // This code is NOT reentrant because qsort makes it too hard
    innercomparator = xcomparator;
    std::sort(xkeys.begin(), xkeys.end(), wrapcomparator);
}


bool UniConf::SortedIterBase::next()
{
    if (index >= count)
        return false;
    current = xkeys[index];
    index += 1;
    return true;
}
