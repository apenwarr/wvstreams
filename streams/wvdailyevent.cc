/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A simple class that can trigger an event on a timed basis.
 *   a) if given an hour, triggers once per day, on that hour.
 *   b) if given a number of times per day, triggers that many times per
 *      day, evenly, starting at the hour given in (a).  (Needed to get a
 *      Microbackup going every 15 minutes.)  
 *
 * Presently has a one-hour granularity in the first case, but that can be
 * extended one day when someone cares.
 *
 */
#include "wvstream.h"
#include "wvdailyevent.h"

#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

WvDailyEvent::WvDailyEvent(int _first_hour, int _num_per_day)
{
    need_reset = false;
    last_hour = -1;
    last_second = -1;
    configure(_first_hour, _num_per_day);
}


// we're "ready" if the time just changed to "first_hour" o'clock,
// OR if the time just changed to "first_hour" o'clock plus a multiple of
// 24*60 / num_per_day seconds.
bool WvDailyEvent::pre_select(SelectInfo &si)
{
    time_t now;
    struct tm *tnow;
    
    now  = time(NULL);
    tnow = localtime(&now);
    
    // too soon after configuration - skip the event
    if (now < not_until)
	return false;

    // for a specific hour
    if (tnow->tm_hour == first_hour)
    {
        if ((first_hour-1) % 24 == last_hour)
            need_reset = true;
    }
    last_hour = tnow->tm_hour;

    // for a number of times a day
    // use the daily "first_hour" as an offset.  (if first_hour is 3, and
    // num_per_day is 2, we want to tick at 3 am and 3 pm.)
    int this_second = ((tnow->tm_hour - first_hour) % 24) * 3600 + tnow->tm_min * 60 + tnow->tm_sec;
    if (num_per_day)
    {
        int seconds_between = 24*60*60 / num_per_day;
        if ((this_second % seconds_between) == 0)
	{
            if (last_second != this_second)
                need_reset = true;
        }
    }
    last_second = this_second;

    return need_reset;
}


bool WvDailyEvent::post_select( SelectInfo& si )
{
    return( need_reset );
}


void WvDailyEvent::execute()
{
    WvStream::execute();
    reset();
}


void WvDailyEvent::reset()
{
    need_reset = false;
}


bool WvDailyEvent::isok() const
{
    return true;
}

void WvDailyEvent::set_num_per_day(int _num_per_day) 
{
    num_per_day = _num_per_day;
    if (num_per_day < 0)
	num_per_day = 1;

    if (num_per_day > 24*60*60)
        num_per_day = 24*60*60;
        
    time_t max = num_per_day ? (24*60*60)/num_per_day : 6*60*60;
    if (max > 6*60*60)
	max = 6*60*60; // unless that's a very long time, 6 hrs

    // don't start until at least one period has gone by
    not_until = time(NULL) + max;
}

void WvDailyEvent::configure(int _first_hour, int _num_per_day)
{
    first_hour = _first_hour;

    // Don't let WvDailyEvents occur more than once a minute. -- use an alarm
    // instead
    if (_num_per_day > 24*60)
        _num_per_day = 24*60;

    set_num_per_day(_num_per_day);
}
