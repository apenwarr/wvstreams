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
    size_t _chunksize) : WvStreamClone(_cloned),
    closecb_func(NULL), closecb_data(NULL)
{
    init(_bps, _chunksize);
}


WvSyncStream::WvSyncStream(WvStream *_cloned, bool _owner, int _srate,
    int _bits, int _msec) : WvStreamClone(_cloned),
    closecb_func(NULL), closecb_data(NULL)
{
    size_t _bps = _srate * _bits / 8;
    size_t _chunksize = _bps * _msec / 1000;
    init(_bps, _chunksize);
    disassociate_on_close = ! _owner;
}


WvSyncStream::~WvSyncStream()
{
    close();
}


void WvSyncStream::init(size_t _bps, size_t _chunksize)
{
    bps = _bps;
    chunksize = _chunksize;
    lowwater = chunksize / 3;
    highwater = chunksize * 2 / 3;
    
    // make full chunk available immediately
    availchunk = chunksize;
    gettimeofday(& lastpoll, NULL);

    if (cloned)
        cloned->force_select(true, false, false);
}


void WvSyncStream::close()
{
    WvStreamClone::close();
    if (closecb_func)
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
    return len;
}


bool WvSyncStream::post_select(SelectInfo &si)
{
    bool havedata = WvStreamClone::post_select(si);
    havedata = true;
    if (si.wants.readable)
    {
        poll();
        if (availchunk < lowwater)
        {
            // not enough data to care about right now
            alarm((highwater - availchunk) * 1000 / bps);
            return false;
        }
    }
    return havedata;
}


void WvSyncStream::poll()
{
    // how long has it been?
    struct timeval now;
    gettimeofday(& now, NULL);
    time_t msec = msecdiff(now, lastpoll);
    if (msec < 5)
        return; // not precise enough to care yet
    lastpoll = now;

    // how much can we read?
    size_t chunk = bps * msec / 1000;
    availchunk += chunk;
    if (availchunk > chunksize)
        availchunk = chunksize; // must have gotten behind in reading
}
