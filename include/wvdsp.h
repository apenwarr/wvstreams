/*
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

/**
 * Class to access the /dev/dsp device in a way that's sane enough for
 * full-duplex access. This is still rather heavily under construction at
 * this time, so please don't count on any side-effects to make anything
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
     */
    WvDsp(int msec_latency, int srate, int bits, bool stereo,
	  bool readable, bool writable);
    virtual ~WvDsp();

    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual bool pre_select(SelectInfo &si);

    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual bool post_select(SelectInfo &si);

    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual size_t uread(void *buf, size_t len);

    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual size_t uwrite(const void *buf, size_t len);


    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual bool isok() const;

    /**
     * Reimplemented from @ref WvStreams for internal reasons
     */
    virtual void close();

    size_t ispace();
    size_t ospace();
    void realtime();

private:
    bool setioctl(int ctl, int param);
    void subproc(bool reading, bool writing);
    
    size_t do_uread(void *buf, size_t len);
    size_t do_uwrite(const void *buf, size_t len);
    
    size_t frag_size, num_frags;
    WvLog log;
    WvMagicCircle rbuf, wbuf;
    WvLoopback rloop, wloop;
    int fd;
};


#endif // __WVDSP_H
