/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * A simple class that can trigger an event once per day.
 * Presently has a one-hour granularity, but that can be extended one
 * day when someone cares.
 *
 */

#include "wvstream.h"
#include "wvdailyevent.h"

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

WvDailyEvent::WvDailyEvent( int _hour, int _num_per_day )
/*******************************************************/
: hour( _hour ), num_per_day( _num_per_day )
{
    need_reset = false;
}

bool WvDailyEvent::select_setup( SelectInfo& si )
/***********************************************/
// we're "ready" if the time just changed to "hour" o'clock.
{
    static int last_hour = -1;
    static int last_minute = -1;

    time_t      now;
    struct tm * tnow;

    now  = time( NULL );
    tnow = localtime( &now );

    // for a specific hour
    if( tnow->tm_hour == hour ) {
        if( (hour-1) % 24 == last_hour )
            need_reset = true;
    }
    last_hour = tnow->tm_hour;

    // for a number of times a day
    int this_minute = tnow->tm_hour*60 + tnow->tm_min;
    if( num_per_day ) {
        int min_between = 24*60 / num_per_day;
        if( this_minute % min_between == 0 ) {
            if( last_minute != this_minute )
                need_reset = true;
        }
    }
    last_minute = this_minute;

    return( need_reset );
}

bool WvDailyEvent::test_set( SelectInfo& si )
/*******************************************/
{
    return( need_reset );
}

void WvDailyEvent::execute()
/**************************/
{
    reset();
}

void WvDailyEvent::reset()
/************************/
{
    need_reset = false;
}

bool WvDailyEvent::isok() const
/*****************************/
{
    return( true );
}

void WvDailyEvent::configure( int _hour, int _num_per_day )
/*********************************************************/
{
    hour = _hour;
    num_per_day = _num_per_day;
}
