/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 */ 
#ifndef __WVTIMESTREAM_H
#define __WVTIMESTREAM_H

#include "wvstream.h"
#include <sys/time.h>

/**
 * WvTimeStream causes select() to be true after a configurable number
 * of milliseconds.  Because programs using WvStream make no guarantees
 * about how often select() will be called, WvTimeStream tries to adjust
 * its timing to a correct _average_ number of milliseconds per tick.
 * 
 * For example, if ms_per_tick=100, WvTimeStream will tick 10 times in one
 * second.  However, there may be a few milliseconds of difference
 * ("jitter") for each individual tick, due to random system delays.
 */
class WvTimeStream : public WvStream
{
    time_t ms_per_tick;

public:
    WvTimeStream();
    
    /**
     * every 'msec' milliseconds, select() will return true on this stream.
     * if 'msec' is 0, the timer is disabled.
     */
    void set_timer(time_t msec);

    virtual bool isok() const;
    
    /**
     * notify timestream that we have "ticked" once
     */
    void tick();
    virtual void execute();
};


#endif // __WVTIMESTREAM_H
