/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
 * 
 * WvTimeStream causes select() to be true after a configurable number
 * of milliseconds.  Because programs using WvStream make no guarantees
 * about how often select() will be called, WvTimeStream tries to adjust
 * its timing to a correct _average_ number of milliseconds per tick.
 * 
 * For example, if ms_per_tick=100, WvTimeStream will tick 10 times in one
 * second.  However, there may be a few milliseconds of difference
 * ("jitter") for each individual tick, due to random system delays.
 * 
 * Note that the time delay given to list.select() is very important,
 * because it determines the granularity of the timing.  select(1000) will
 * allow ticks only every 1000 ms.  If ms_per_tick=100, then WvTimeStream
 * will tick about 10 times (all at once) every second. select(-1)
 * (wait forever) may never tick at all -- be careful.
 */
#ifndef __WVTIMESTREAM_H
#define __WVTIMESTREAM_H

#include "wvstream.h"
#include <sys/time.h>

class WvTimeStream : public WvStream
{
    struct timeval last_tv;
    int ms_per_tick, max_backlog;

public:
    WvTimeStream();
    
    // every 'msec' milliseconds, select() will return true on this stream.
    // if 'msec' is 0, the timer is disabled.
    void set_timer(int msec, int max_backlog = 10);

    virtual bool isok() const;
    virtual bool select_setup(SelectInfo &si);
    virtual bool test_set(SelectInfo &si);
    
    // notify timestream that we have "ticked" once
    void WvTimeStream::tick();
    virtual void execute();
};


#endif // __WVTIMESTREAM_H
