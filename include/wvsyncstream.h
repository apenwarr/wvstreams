/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A throttled stream.
 */
#ifndef __WVSYNCSTREAM_H
#define __WVSYNCSTREAM_H

#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>
#include "wvstream.h"
#include "wvstreamclone.h"

/**
 * WvSyncStream throttles its input to the specified bitrate.
 *
 * It only becomes readable at periodic intervals.
 */
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
     *
     * "cloned" is the stream to wrap
     * "bps" is the number of bytes per second to allow
     * "avgchunk" is the average number of bytes to process at once
     * "maxchunk" is the maximum number of bytes to process at once
     */
    WvSyncStream(WvStream *cloned, size_t bps,
        size_t avgchunk, size_t maxchunk);
    virtual ~WvSyncStream();

    /**
     * Convenience constructor for throttling monaural audio streams.
     * 
     * "cloned" is the stream to wrap
     * "owner" is if false, sets disassociate_on_close
     * "srate" is the sampling rate in Hz
     * "bits" is the number of bits per sample
     * "msec" is the allowable average latency
     * @deprecated
     */
    WvSyncStream(WvStream *cloned, bool owner, int srate, int bits,
        int msec = 10);
       
    virtual size_t uread(void *buf, size_t count);
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);

private:
    void init(size_t _bps, size_t _avgchunk, size_t _maxchunk);
    
    // updates availchunk to reflect the max amount of data available now
    void poll();
    // resets the timing information
    void resettimer();
};

#endif  // __WVSYNCSTREAM_H
