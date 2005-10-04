/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
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
#include "wvdailyevent.h"
#include "wvstream.h"
#include "wvtimeutils.h"

#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#define NUM_MINS_IN_DAY (24*60)
#define NUM_SECS_IN_DAY (60*NUM_MINS_IN_DAY)

WvDailyEvent::WvDailyEvent(int _first_hour, int _num_per_day, bool _skip_first)
    : prev(time(NULL))
{
    configure(_first_hour, _num_per_day, _skip_first);
}


// Compute the next time this stream should select()
bool WvDailyEvent::pre_select(SelectInfo &si)
{
    WvTime next(next_event(), 0);
    if (next)
	si.msec_timeout = msecdiff(next, wvtime());
    return WvStream::pre_select(si);
}


// Test to see if the timer has gone off
bool WvDailyEvent::post_select(SelectInfo& si)
{
    bool timer_rang = false;
    WvTime next(next_event(), 0);
    if (next < wvtime())
    {
	timer_rang = true;
	prev = next;
    }

    return WvStream::post_select(si) || timer_rang;
}


void WvDailyEvent::set_num_per_day(int _num_per_day) 
{
    num_per_day = _num_per_day;
    if (num_per_day < 0)
	num_per_day = 1;

    if (num_per_day > NUM_SECS_IN_DAY)
        num_per_day = NUM_SECS_IN_DAY;
        
    time_t max = num_per_day ? NUM_SECS_IN_DAY/num_per_day : 6*60*60;
    if (max > 6*60*60)
	max = 6*60*60; // unless that's a very long time, 6 hrs

    // don't start until at least one period has gone by
    prev = time(NULL);
    not_until = prev + max;
}


void WvDailyEvent::configure(int _first_hour, int _num_per_day, bool _skip_first)
{
    first_hour = _first_hour;
    skip_first = _skip_first;

    // Don't let WvDailyEvents occur more than once a minute. -- use an alarm
    // instead
    if (_num_per_day > NUM_MINS_IN_DAY)
        _num_per_day = NUM_MINS_IN_DAY;

    set_num_per_day(_num_per_day);
}

// the daily event occurs each day at first_hour on the hour, or at
// some multiple of the interval *after* that hour.
time_t WvDailyEvent::next_event() const
{
    if (!num_per_day) // disabled
	return 0;

    assert(prev);
    
    time_t interval = NUM_SECS_IN_DAY/num_per_day;
    time_t start = prev + interval;
   
    // find the time to start counting from (up to 24 hours in the past)
    struct tm *tm = localtime(&start);
    if (tm->tm_hour < first_hour)
    {
	start = prev - NUM_SECS_IN_DAY + 1; // this time yesterday
	tm = localtime(&start);
    }
    tm->tm_hour = first_hour; // always start at the given hour
    tm->tm_min = tm->tm_sec = 0; // right on the hour
    start = mktime(tm); // convert back into a time_t

    // find the next event after prev that's a multiple of 'interval'
    // since 'start'
    time_t next = prev + interval;
    if ((next - start)%interval != 0)
	next = start + (next - start)/interval * interval;
    
    assert(next);
    assert(next > 100000);

    while (skip_first && next < not_until)
	next += interval;

    return next;
}
