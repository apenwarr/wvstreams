/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * WvSyncStream throttles its input to the specified bitrate.
 * It only becomes readable at periodic intervals.
 */
#include "wvsyncstream.h"
#include "wvtimeutils.h"

WvSyncStream::WvSyncStream(WvStream *_cloned, size_t _bps,
    size_t _avgchunk, size_t _maxchunk) :
    WvStreamClone(_cloned),
    closecb_func(0), closecb_data(NULL)
{
    init(_bps, _avgchunk, _maxchunk);
}


WvSyncStream::WvSyncStream(WvStream *_cloned, bool _owner, int _srate,
    int _bits, int _msec) : WvStreamClone(_cloned),
    closecb_func(0), closecb_data(NULL)
{
    size_t _bps = _srate * _bits / 8;
    size_t _avgchunk = _bps * _msec / 1000;
    init(_bps, _avgchunk, _avgchunk * 5); // arbitrary choice
    disassociate_on_close = ! _owner;
}


WvSyncStream::~WvSyncStream()
{
    close();
}


void WvSyncStream::init(size_t _bps, size_t _avgchunk, size_t _maxchunk)
{
    bps = _bps;
    avgchunk = _avgchunk;
    maxchunk = _maxchunk;
    // allow +/- 50% tolerance
    // FIXME: this is a purely arbitrary number
    int tol = avgchunk / 2;
    lowater = avgchunk - tol;
    hiwater = avgchunk + tol;
    waiting = false;
    resettimer();

    if (cloned)
        cloned->force_select(true, false, false);
}


void WvSyncStream::close()
{
    WvStreamClone::close();
    if (!! closecb_func)
        closecb_func(*this, closecb_data);
}


size_t WvSyncStream::uread(void *buf, size_t count)
{
    poll();
    if (count > availchunk)
        count = availchunk;
    if (availchunk == 0)
        return 0; // try again later

    size_t len = WvStreamClone::uread(buf, count);
    availchunk -= len;
    usedchunk += len;
    return len;
}


bool WvSyncStream::pre_select(SelectInfo &si)
{
    if (waiting)
    {
        poll();
        if (availchunk < lowater)
        {
            time_t timeout = (hiwater - availchunk) * 1000 / bps + 1;
            if (timeout > 0)
            {
                if (timeout < si.msec_timeout || si.msec_timeout < 0)
                    si.msec_timeout = timeout;
                return false;
            }
        }
        waiting = false;
        return true; // we know we had data
    }
    return WvStreamClone::pre_select(si);
}


bool WvSyncStream::post_select(SelectInfo &si)
{
    bool havedata = WvStreamClone::post_select(si);
    if (havedata && si.wants.readable)
    {
        poll();
        if (availchunk < lowater)
        {
            // not enough data to care about right now
            waiting = true;
            return false;
        }
    }
    return havedata;
}


void WvSyncStream::poll()
{
    // how long has it been since we started
    struct timeval now;
    gettimeofday(& now, NULL);
    time_t msec = msecdiff(now, reference);
    if (msec > 60000)
    {
        // avoid overflow by adjusting reference time
        reference.tv_sec -= 30;
        msec -= 30000;
        size_t consume = 30 * bps;
        if (usedchunk >= consume)
            usedchunk -= consume;
        else
            usedchunk = 0; // got very far behind reading?
    }
    else if (msec < 0)
    {
        // reference clock is confused!
        resettimer();
        return;
    }
    
    // how much can we read?
    size_t totalchunk = bps * msec / 1000;
    availchunk = totalchunk - usedchunk;
    if (availchunk > maxchunk)
    {
        // resynchronize after a long delay
        availchunk = maxchunk;
        usedchunk = totalchunk - availchunk;
    }
}


void WvSyncStream::resettimer()
{
    // make full chunk available immediately
    gettimeofday(& reference, NULL);
    reference.tv_sec -= 1;
    availchunk = hiwater;
    usedchunk = bps - availchunk;
}
