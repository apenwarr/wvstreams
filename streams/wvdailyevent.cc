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

WvDailyEvent::WvDailyEvent( int _hour )
/*************************************/
: hour( _hour )
{
    need_reset = false;
}

bool WvDailyEvent::select_setup( SelectInfo& si )
/***********************************************/
// we're "ready" if the time just changed to "hour" o'clock.
{
    static int last_hour = -1;

    time_t      now;
    struct tm * tnow;
    bool        ret = false;

    now  = time( NULL );
    tnow = localtime( &now );

    if( tnow->tm_hour == hour ) {
        if( (hour-1) % 24 == last_hour ) {
            ret = true;
            need_reset = true;
        }
    }

    last_hour = tnow->tm_hour;
    return( need_reset || ret );
}

bool WvDailyEvent::test_set( SelectInfo& si )
/*******************************************/
// I imagine this doesn't much matter.
{
    return( false );
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

void WvDailyEvent::set_hour( int _hour )
/**************************************/
{
    hour = _hour;
}
