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
    prev = time(NULL);
    configure(_first_hour, _num_per_day);
}


// we're ready if now is later than the next scheduled event
bool WvDailyEvent::pre_select(SelectInfo &si)
{
    if (num_per_day && !need_reset)
    {
	time_t now = time(NULL), next = next_event();
	assert(prev);
	assert(next);
	assert(prev > 100000);
	assert(next > 100000);
	if (now >= next)
	{
	    need_reset = true;
	    prev = next;
	}
    }
    bool ret = WvStream::pre_select(si) || need_reset;
    // printf("%p ret=%d msd=%d\n", this, ret, si.msec_timeout);
    return ret;
}


bool WvDailyEvent::post_select(SelectInfo& si)
{
    return need_reset;
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
    prev = time(NULL);
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


// the daily event occurs each day at first_hour on the hour, or at
// some multiple of the interval *after* that hour.
time_t WvDailyEvent::next_event() const
{
    if (!num_per_day) // disabled
	return 0;
    
    time_t start, now, next, interval = 24*60*60/num_per_day;
    struct tm *tm;
    
    assert(prev);
    start = prev + interval;
    
    // find the time to start counting from (up to 24 hours in the past)
    tm = localtime(&start);
    if (tm->tm_hour < first_hour)
    {
	start = now - 24*60*60; // this time yesterday
	tm = localtime(&start);
    }
    tm->tm_hour = first_hour; // always start at the given hour
    tm->tm_min = tm->tm_sec = 0; // right on the hour
    start = mktime(tm); // convert back into a time_t
    
    // find the next event after 'now' that's a multiple of 'interval'
    // since 'start'
    next = prev + interval;
    if ((next - start)%interval != 0)
	next = start + (next - start)/interval * interval + interval;
    
    // too soon after configuration - skip the event
    assert(next);
    assert(next > 100000);
    while (next < not_until)
    	next += interval;

    return next;
}
