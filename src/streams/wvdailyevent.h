/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A simple class that can trigger an event on a timed basis.
 *   - if given an hour, triggers once per day, on that hour.
 *   - if given a number of times per day, triggers that many times per
 *     day, evenly, starting at midnight.  (Needed to get a Microbackup
 *     going every 15 minutes.)  
 *
 * Presently has a one-hour granularity in the first case, but that can be
 * extended one day when someone cares.
 *
 */

#ifndef __WVDAILYEVENT_H
#define __WVDAILYEVENT_H

#include "wvstream.h"

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
