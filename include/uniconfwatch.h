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
#include "wvlog.h"

class UniConf;

// parameters are: UniConf object, depth, userdata
DeclareWvCallback(3, void, UniConfCallback, const UniConf &,
    UniConfDepth::Type, void *);

/**
 * Observes a particular key.
 */
class UniConfWatch
{
protected:
    UniConfWatch() { }
    UniConfWatch(const UniConfWatch &) { }

public:
    virtual ~UniConfWatch() { }

    /**
     * Called when a key changes in some way.
     * "key" is the key
     * "depth" is the recursion depth
     */
    virtual void notify(const UniConf &key, UniConfDepth::Type depth) = 0;
};


/**
 * A UniConfWatch that sets a boolean flag on notify.
 */
class UniConfWatchSetBool : public UniConfWatch
{
    bool *xflag;

public:
    UniConfWatchSetBool(bool *flag);

    /***** Overridden methods *****/
    virtual void notify(const UniConf &key, UniConfDepth::Type depth);
};


/**
 * A UniConfWatch that logs a message on notify.
 * For debugging purposes.
 */
class UniConfWatchLog : public UniConfWatch
{
    WvLog &log;

public:
    UniConfWatchLog(WvLog &log);
    
    /***** Overridden methods *****/
    virtual void notify(const UniConf &key, UniConfDepth::Type depth);
};


/**
 * A UniConfWatch that invokes a callback.
 */
class UniConfWatchCallback : public UniConfWatch
{
    UniConfCallback xcallback;
    void *xuserdata;

public:
    UniConfWatchCallback(const UniConfCallback &callback, void *userdata);
    
    /***** Overridden methods *****/
    virtual void notify(const UniConf &key, UniConfDepth::Type depth);
};

#endif //__UNICONFWATCH_H
