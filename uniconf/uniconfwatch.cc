/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf notification support code.
 */
#include "uniconfwatch.h"
#include "uniconf.h"

/***** UniConfWatch *****/



/***** UniConfWatchSetBool *****/

UniConfWatchSetBool::UniConfWatchSetBool(bool *flag) :
    xflag(flag)
{
}


void UniConfWatchSetBool::notify(const UniConf &key, UniConfDepth::Type depth)
{
    *xflag = true;
}



/***** UniConfWatchLog *****/

UniConfWatchLog::UniConfWatchLog(WvLog &log) :
    log(log)
{
}


void UniConfWatchLog::notify(const UniConf &key, UniConfDepth::Type depth)
{
    log("key changed: %s (depth %s)\n", key.fullkey(),
        UniConfDepth::nameof(depth));
}



/***** UniConfWatchCallback *****/

UniConfWatchCallback::UniConfWatchCallback(
    const UniConfCallback &callback, void *userdata) :
    xcallback(callback), xuserdata(userdata)
{
}


void UniConfWatchCallback::notify(const UniConf &key, UniConfDepth::Type depth)
{
    xcallback(key, depth, xuserdata);
}
