/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * WvSyncStream throttles its input to the specified bitrate.
 * It only becomes readable at periodic intervals.
 */
#ifndef __WVSYNCSTREAM_H
#define __WVSYNCSTREAM_H

#include <sys/time.h>
#include <time.h>
#include "wvstream.h"
#include "wvstreamclone.h"

class WvSyncStream : public WvStreamClone
{
    size_t bps;
    size_t avgchunk;
    size_t maxchunk;
    size_t availchunk;
    size_t usedchunk;
    size_t lowater, hiwater; // controls latency
    bool waiting;
    
    struct timeval reference; // last reference time taken

public:
    /**
     * Creates a new WvSyncStream.
     *   _cloned    : the stream to wrap
     *   _bps       : the number of bytes per second to allow
     *   _avgchunk  : the average number of bytes to process at once
     *   _maxchunk  : the maximum number of bytes to process at once
     */
    WvSyncStream(WvStream *_cloned, size_t _bps,
        size_t _avgchunk, size_t _maxchunk);
    virtual ~WvSyncStream();

    // Remove me: for compatibility with existing stream audio apps
    // FIXME: how does this deal with stereo & various interleaving formats? 
    WvSyncStream(WvStream *_cloned, bool _owner, int _srate, int _bits,
        int _msec = 10);
    
    /**
     * Sets a callback to be invoked on close().
     */
    void setclosecallback(WvStreamCallback _callfunc, void *_userdata)
       { closecb_func = _callfunc; closecb_data = _userdata; }
       
    virtual void close();
    virtual size_t uread(void *buf, size_t count);
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);

protected:
    // close callback
    WvStreamCallback closecb_func;
    void *closecb_data;

private:
    void init(size_t _bps, size_t _avgchunk, size_t _maxchunk);
    
    // updates availchunk to reflect the max amount of data available now
    void poll();
    // resets the timing information
    void resettimer();
};

#endif  // __WVSYNCSTREAM_H
