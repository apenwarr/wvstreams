/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * One more attempt at making a decent stream for Linux /dev/dsp.  This is
 * apparently much trickier than it looks.
 */
#ifndef __WVDSP_H
#define __WVDSP_H

#include "wvmagiccircle.h"
#include "wvloopback.h"
#include "wvlog.h"
#include "wvrateadjust.h"

/**
 * Class to access the /dev/dsp device in a way that's sane enough for
 * full-duplex access.
 * 
 * This is still rather heavily under construction at this time, so
 * please don't count on any side-effects to make anything
 * built with this class to work.
 */
class WvDsp : public WvStream
{
public:

    /**
     * Construct a /dev/dsp accessor object
     * msec_latency = number of milliseconds of latency that are permissible
     * srate = sampling rate ( 44, 22, 11, or 8 Hz )
     * bits = bits per sample ( 8 or 16 )
     * stereo = should this be a stereo stream?
     * readable/writeable = should this stream be readable and/or writeable
     * realtime = should the stream give itself realtime priority (needs root)
     * oss = is this a real OSS driver (not ALSA's OSS emulation)?
     */
    WvDsp(int msec_latency, int srate, int bits, bool stereo,
	  bool readable = true, bool writable = true,
          bool _realtime = false, bool _oss = false);
    virtual ~WvDsp();

    size_t ispace();
    size_t ospace();
    void realtime();
    
    /*** Overridden members ***/
    
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);
    virtual bool isok() const;
    virtual void close();

private:
    bool setioctl(int ctl, int param);
    void subproc(bool reading, bool writing);

    size_t do_uread(void *buf, size_t len);
    size_t do_uwrite(const void *buf, size_t len);
    
    size_t frag_size, num_frags;
    WvLog log;
    WvMagicCircle rcircle, wcircle;
    WvDynBuf rbuf, wbuf;
    WvLoopback rloop, wloop;
    WvRateAdjust inrate, outrate;
    int fd;
    bool is_realtime;
};


#endif // __WVDSP_H
