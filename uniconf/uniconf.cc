/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a hierarchical registry abstraction.
 */
#include "uniconf.h"
#include "uniconfroot.h"
#include "uniconfgen.h"
#include "wvstream.h"
#include <assert.h>

/***** UniConf *****/

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
    if (! value.isnull())
    {
        // try to recognize an integer
        if (* value.cstr())
        {
            char *end;
            int num = strtol(value.cstr(), & end, 0);
            if (!*end)
                return num; // was a valid integer
        }
        
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


void UniConf::unmount(bool commit) const
{
    return xroot->_unmount(xfullkey, commit);
}


bool UniConf::ismountpoint() const
{
    // FIXME: this test does not work if the key is not provided by
    //        the root of a mountpoint
    UniConfKey mountpoint;
    
    UniConfGen *gen = whichmount(&mountpoint);
    if (gen && mountpoint == fullkey())
	return true;
    else
	return false;
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
        if (everything || !! value)
            stream.print("%s = %s\n", it->fullkey(), value);
    }
}



/***** UniConf::Iter *****/

UniConf::Iter::Iter(const UniConf &root) 
    : KeyIterBase(root),
    it(*root.rootobj(), root.fullkey())
{
}


void UniConf::Iter::rewind()
{
    it.rewind();
}


bool UniConf::Iter::next()
{
    if (it.next())
    {
        xcurrent = xroot[it.key()];
        return true;
    }
    return false;
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(const UniConf &root,
    UniConfDepth::Type depth) :
    KeyIterBase(root),
    top(root), depth(depth)
{
}


void UniConf::RecursiveIter::rewind()
{
    itlist.zap();
    first = false;
    switch (depth)
    {
        case UniConfDepth::ZERO:
            first = true;
            break;

        case UniConfDepth::ONE:
        case UniConfDepth::INFINITE:
            first = true;
            // fall through

        case UniConfDepth::CHILDREN:
        case UniConfDepth::DESCENDENTS:
            itlist.append(& top, false);
            top.rewind();
            break;
    }
}


bool UniConf::RecursiveIter::next()
{
    if (first)
    {
        first = false;
        xcurrent = xroot;
        return true;
    }

    UniConf::IterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::Iter &it = itlistit();
        if (it.next())
        {
            xcurrent = *it;
            if ((depth == UniConfDepth::INFINITE ||
            depth == UniConfDepth::DESCENDENTS) &&
                xcurrent.haschildren())
            {
                UniConf::Iter *subit = new UniConf::Iter(xcurrent);
                subit->rewind();
                itlist.prepend(subit, true);
            }
            return true;
        }
        itlistit.xunlink();
    }
    return false;
}



/***** UniConf::PatternIter *****/

UniConf::PatternIter::PatternIter(const UniConf &root,
    const UniConfKey &pattern) :
    KeyIterBase(root),
    xpattern(pattern), it(NULL)
{
}


UniConf::PatternIter::~PatternIter()
{
    delete it;
}


void UniConf::PatternIter::rewind()
{
    done = false;
    if (xpattern.iswild())
    {
        if (! it)
            it = new UniConf::Iter(root());
        it->rewind();
    }
}


bool UniConf::PatternIter::next()
{
    // handle wildcards
    if (it)
    {
        while (it->next())
        {
            xcurrent = **it;
            if (xcurrent.key().matches(xpattern))
                return true;
        }
        return false;
    }
    // handle isolated elements quickly
    if (done)
        return false;
    done = true;
    xcurrent = root()[xpattern];
    return xcurrent.exists();
}



/***** UniConf::XIter *****/

UniConf::XIter::XIter(const UniConf &root,
    const UniConfKey &pattern) :
    KeyIterBase(root),
    xpattern(pattern)
{
}


void UniConf::XIter::rewind()
{
    itlist.zap();
    if (! xpattern.isempty())
    {
        UniConf::PatternIter *subit = new UniConf::PatternIter(
            root(), xpattern.first());
        subit->rewind();
        itlist.prepend(subit, true);
    }
}


bool UniConf::XIter::next()
{
    UniConf::PatternIterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::PatternIter &it = itlistit();
        if (it.next())
        {
            // return key if we reached the desired depth
            xcurrent = *it;
            int depth = itlist.count();
            int desired = xpattern.numsegments();
            if (depth == desired)
                return true;
            
            // otherwise add a level to the stack
            UniConf::PatternIter *subit = new UniConf::PatternIter(
                it(), xpattern.segment(depth));
            subit->rewind();
            itlist.prepend(subit, true);
            itlistit.rewind();
            continue;
        }
        itlistit.xunlink();
    }
    return false;
}



/***** UniConf::SortedKeyIterBase *****/

UniConf::SortedKeyIterBase::SortedKeyIterBase(const UniConf &root,
    UniConf::SortedKeyIterBase::Comparator comparator) 
    : KeyIterBase(root), xcomparator(comparator), xkeys(true)
{
}


UniConf::SortedKeyIterBase::~SortedKeyIterBase()
{
    _purge();
}


int UniConf::SortedKeyIterBase::defcomparator(const UniConf &a,
					      const UniConf &b)
{
    return a.fullkey().compareto(b.fullkey());
}


UniConf::SortedKeyIterBase::Comparator
    UniConf::SortedKeyIterBase::innercomparator = NULL;

int UniConf::SortedKeyIterBase::wrapcomparator(const UniConf **a,
					       const UniConf **b)
{
    return innercomparator(**a, **b);
}


void UniConf::SortedKeyIterBase::_purge()
{
    count = xkeys.count();
    xkeys.zap();
}


void UniConf::SortedKeyIterBase::_rewind()
{
    index = 0;
    count = xkeys.count();
    
    // This code is NOT reentrant because qsort makes it too hard
    innercomparator = xcomparator;
    qsort(xkeys.ptr(), count, sizeof(UniConf*),
        (int (*)(const void *, const void *))wrapcomparator);
}


bool UniConf::SortedKeyIterBase::next()
{
    if (index >= count)
        return false;
    xcurrent = *xkeys[index];
    index += 1;
    return true;
}
