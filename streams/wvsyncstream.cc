/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * WvSyncStream throttles its input to the specified bitrate.
 * It only becomes readable at periodic intervals.
 */
#include "wvsyncstream.h"


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
    init(_bps, _bps * _msec / 1000);
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
    wvcon->print("read %s bytes\n", len);
    availchunk -= len;
    return len;
}


bool WvSyncStream::post_select(SelectInfo &si)
{
    bool havedata = WvStreamClone::post_select(si);
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
    unsigned long sec = now.tv_sec - lastpoll.tv_sec;
    unsigned long usec = now.tv_usec - lastpoll.tv_usec;
    lastpoll = now;

    // how much can we read?
    size_t chunk = size_t(bps * sec) +
        size_t((long long)bps * (long long)usec / 1000000LL);
    availchunk += chunk;
    if (availchunk > chunksize)
        availchunk = chunksize; // must have gotten behind in reading
}
