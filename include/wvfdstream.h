/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base class for streams built on Unix file descriptors.
 */ 
#ifndef __WVFDSTREAM_H
#define __WVFDSTREAM_H

#include "wvstream.h"

class WvFDStream : public WvStream
{
protected:
    int rfd;
    int wfd;

    void setfd(int fd)
        { rfd = wfd = fd; }

public:
    /**
     * constructor to create a WvStream from an existing file descriptor.
     * The file descriptor is closed automatically by the destructor.  If
     * this is undesirable, duplicate it first using dup().
     */
    WvFDStream(int _rwfd = -1);
    WvFDStream(int _rfd, int _wfd);
    virtual ~WvFDStream();

    /**
     * return the Unix file descriptor for reading from this stream
     */
    int getrfd() const
        { return rfd; }
    
    /**
     * return the Unix file descriptor for writing to this stream
     */
    int getwfd() const
        { return wfd; }

    /**
     * return the Unix file descriptor for reading and writing only
     * if they are the same, else asserts!
     */
    int getfd() const
        { assert(rfd == wfd); return rfd; }

    /**
     * shuts down the reading or writing sides of the stream
     */
    void noread();
    void nowrite();

    /***** Overridden members *****/
    virtual void close();
    virtual bool isok() const;
    virtual size_t uread(void *buf, size_t count);
    virtual size_t uwrite(const void *buf, size_t count);
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
};

#endif // __WVFDSTREAM_H
