/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * A simple class that can trigger an event once per day.
 * Presently has a one-hour granularity, but that can be extended one
 * day when someone cares.
 *
 */

#ifndef __WVDAILYEVENT_H
#define __WVDAILYEVENT_H

#include "wvstream.h"

class WvDailyEvent : public WvStream
/**********************************/
{
public:
    WvDailyEvent( int _hour );

    virtual bool select_setup( SelectInfo& si );
    virtual bool test_set( SelectInfo& si );

    // execute() and any overridden versions of it must call reset().
    virtual void execute();
    void         reset();

    virtual bool isok() const;

    void set_hour( int _hour );

private:
    int     hour;
    bool    need_reset;
};

#endif
