/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConf notification support code.
 */
#include "uniconfwatch.h"
#include "uniconf.h"

/***** UniWatchManager *****/

UniWatchManager::UniWatchManager()
{
}


UniWatchManager::~UniWatchManager()
{
}


void UniWatchManager::add_callback(const UniConfKey &key,
    const UniConfCallback &callback, void *userdata,
    UniConfDepth::Type depth)
{
}


void UniWatchManager::del_callback(const UniConfKey &key,
    const UniConfCallback &callback, void *userdata,
    UniConfDepth::Type depth)
{
}


void UniWatchManager::add_setbool(const UniConfKey &key, bool *flag,
    UniConfDepth::Type depth)
{
}


void UniWatchManager::del_setbool(const UniConfKey &key, bool *flag,
    UniConfDepth::Type depth)
{
}


void UniWatchManager::delta(const UniConfKey &key)
{
}
