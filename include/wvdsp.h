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


class WvDsp : public WvStream
{
public:
    WvDsp(int msec_latency, int srate, int bits, bool stereo,
	  bool readable, bool writable);
    virtual ~WvDsp();
    
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);

    virtual bool isok() const;
    virtual void close();

public:
    bool setioctl(int ctl, int param);
    void realtime();
    void subproc(bool reading, bool writing);
    
    size_t ispace();
    size_t ospace();
    
    size_t do_uread(void *buf, size_t len);
    size_t do_uwrite(const void *buf, size_t len);
    
    size_t frag_size, num_frags;
    WvLog log;
    WvMagicCircle rbuf, wbuf;
    WvLoopback rloop, wloop;
    int fd;
};


#endif // __WVDSP_H
