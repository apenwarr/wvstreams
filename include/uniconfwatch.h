/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConf notification support code.
 */
#ifndef __UNICONFWATCH_H
#define __UNICONFWATCH_H

#include "uniconfdefs.h"
#include "uniconftree.h"
#include "wvlinklist.h"
#include "wvcallback.h"

class UniConf;

// parameters are: UniConf object, userdata
DeclareWvCallback(2, void, UniConfCallback, const UniConf &, void *);

/**
 * Manages a collection of UniConf notification watchers.
 */
class UniWatchManager
{
public:
    UniWatchManager();
    ~UniWatchManager();
    
    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by invoking a callback.
     */
    void add_callback(const UniConfKey &key,
        const UniConfCallback &callback, void *userdata,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);
    
    /**
     * Cancels notification requested using add_callback().
     */
    void del_callback(const UniConfKey &key,
        const UniConfCallback &callback, void *userdata,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by setting a flag.
     */
    void add_setbool(const UniConfKey &key, bool *flag,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Cancels notification requested using add_setbool().
     */
    void del_setbool(const UniConfKey &key, bool *flag,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Sends notification that a key has changed.
     */
    void delta(const UniConfKey &key);
};

#endif //__UNICONFWATCH_H
