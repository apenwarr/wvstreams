/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * A WvSplitStream uses two different file descriptors: one for input
 * and another for output.  Because of this, simultaneous select()
 * for read and write will not work, and select() via WvStreamList
 * only works for read.
 * 
 * This is primarily used for the combined stdin/stdout stream.
 */
#ifndef __WVSPLITSTREAM_H
#define __WVSPLITSTREAM_H

#include "wvstream.h"


class WvSplitStream : public WvStream
{
public:
    WvSplitStream(int _rfd, int _wfd);
    virtual ~WvSplitStream();
    
    virtual void close();
    virtual int getfd() const;
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);
    virtual bool isok() const;
    virtual bool select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception);
    virtual bool select(time_t msec_timeout, bool readable = true,
			bool writable = false, bool isexception = false);
    int getrfd() const
        { return rfd; }
    int getwfd() const
        { return wfd; }
    
protected:
    int rfd, wfd;
    int in_progress;

    WvSplitStream(); // derived classes might not know the fds yet!
    
    virtual size_t uwrite(const void *buf, size_t size);
    virtual size_t uread(void *buf, size_t size);
};

#endif // __WVSPLITSTREAM_H
