/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
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
    return xmanager->exists(xfullkey);
}


bool UniConf::haschildren() const
{
    return xmanager->haschildren(xfullkey);
}


WvString UniConf::get(WvStringParm defvalue) const
{
    WvString value = xmanager->get(xfullkey);
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
            if (*end)
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
    return xmanager->set(xfullkey, value);
}


bool UniConf::setint(int value) const
{
    return xmanager->set(xfullkey, WvString(value));
}


bool UniConf::zap() const
{
    return xmanager->zap(xfullkey);
}


bool UniConf::refresh(UniConfDepth::Type depth) const
{
    return xmanager->refresh(xfullkey, depth);
}


bool UniConf::commit(UniConfDepth::Type depth) const
{
    return xmanager->commit(xfullkey, depth);
}


UniConfMount UniConf::mount(const UniConfLocation &location,
    bool refresh) const
{
    UniConfGen *gen = xmanager->mount(xfullkey, location);
    if (gen && refresh)
        gen->refresh(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    return UniConfMount(*this, gen);
}


UniConfMount UniConf::mountgen(UniConfGen *gen, bool refresh) const
{
    gen = xmanager->mountgen(xfullkey, gen);
    if (gen && refresh)
        gen->refresh(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    return UniConfMount(*this, gen);
}


bool UniConf::ismountpoint() const
{
    // FIXME: temporary
    UniConfMount mount(whichmount());
    return ! mount.isnull() &&
        mount.mountpoint().fullkey() == fullkey();
    
#if 0
    UniConf::MountIter it(*this);
    it.rewind();
    return it.next();
#endif
}


UniConfMount UniConf::whichmount() const
{
    UniConfKey mountpoint;
    UniConfGen *provider = xmanager->whichmount(xfullkey,
        & mountpoint);
    if (provider)
        return UniConfMount(root()[mountpoint], provider);
    return UniConfMount();
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

UniConf::Iter::Iter(const UniConf &root) :
    UniConfRoot::Iter(*root.manager(), root.fullkey()),
    xroot(root), current(NULL)
{
}


bool UniConf::Iter::next()
{
    if (UniConfRoot::Iter::next())
    {
        current = xroot[key()];
        return true;
    }
    return false;
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(const UniConf &root,
    UniConfDepth::Type depth) :
    xroot(root), top(root), depth(depth), current(NULL)
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
        current = xroot;
        return true;
    }

    IterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::Iter &it = itlistit();
        if (it.next())
        {
            current = *it;
            if ((depth == UniConfDepth::INFINITE ||
            depth == UniConfDepth::DESCENDENTS) &&
                current.haschildren())
            {
                UniConf::Iter *subit = new UniConf::Iter(current);
                subit->rewind();
                itlist.prepend(subit, true);
            }
            return true;
        }
        itlistit.xunlink();
    }
    return false;
}



/***** UniConfMount *****/

bool UniConfMount::isok() const
{
    return xgen && xgen->isok();
}


void UniConfMount::unmount(bool commit)
{
    if (! xgen)
        return;
    if (commit)
        xgen->commit(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    xmountpoint.manager()->unmount(xmountpoint.fullkey(), xgen);
    xgen = NULL;
}
