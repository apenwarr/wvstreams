/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A period event stream.
 */

#ifndef __WVDAILYEVENT_H
#define __WVDAILYEVENT_H

#include "wvstream.h"

/**
 * A simple class that can trigger an event on a timed basis.
 * 
 * The period may be specified the following ways:
 * 
 *  - Given an hour, triggers once per day, on that hour.
 *  - Given a number of times per day, triggers that many times
 *     per day, evenly, starting at the specified hour.
 * 
 * 
 * Presently has a one-hour granularity in the first case, but that
 * can be fixed someday when someone cares.
 * 
 */
class WvDailyEvent : public WvStream
/**********************************/
{
public:
    WvDailyEvent( int _first_hour, int _num_per_day=0 );

    virtual bool pre_select( SelectInfo& si );
    virtual bool post_select( SelectInfo& si );

    // execute() and any overridden versions of it must call reset().
    virtual void execute();
    void         reset();

    virtual bool isok() const;

    void configure( int _first_hour, int _num_per_day=0 );
    void set_hour( int h )
        { configure( h, num_per_day ); }

private:
    int     first_hour;
    int     num_per_day;
    bool    need_reset;
    int     last_hour;
    int     last_minute;
};

#endif
