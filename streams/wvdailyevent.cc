/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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

WvDailyEvent::WvDailyEvent( int _first_hour, int _num_per_day )
/*******************************************************/
: first_hour( _first_hour ), num_per_day( _num_per_day )
{
    need_reset = false;
    last_hour = -1;
    last_minute = -1;
}

bool WvDailyEvent::pre_select( SelectInfo& si )
/***********************************************/
// we're "ready" if the time just changed to "first_hour" o'clock,
// OR if the time just changed to "first_hour" o'clock plus a multiple of
// 24*60 / num_per_day minutes.
{
    time_t      now;
    struct tm * tnow;

    now  = time( NULL );
    tnow = localtime( &now );

    // for a specific hour
    if( tnow->tm_hour == first_hour ) {
        if( (first_hour-1) % 24 == last_hour )
            need_reset = true;
    }
    last_hour = tnow->tm_hour;

    // for a number of times a day
    // use the daily "first_hour" as an offset.  (if first_hour is 3, and
    // num_per_day is 2, we want to tick at 3 am and 3 pm.)
    int this_minute = ( ( tnow->tm_hour - first_hour )%24 )*60 + tnow->tm_min;
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

bool WvDailyEvent::post_select( SelectInfo& si )
/*******************************************/
{
    return( need_reset );
}

void WvDailyEvent::execute()
/**************************/
{
    WvStream::execute();
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

void WvDailyEvent::configure( int _first_hour, int _num_per_day )
/***************************************************************/
{
    first_hour = _first_hour;
    num_per_day = _num_per_day;
}
