/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 * 
 * A WvSplitStream uses two different file descriptors: one for input
 * and another for output.
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
    virtual bool isok() const;
    virtual bool select_setup(SelectInfo &si);
    virtual bool test_set(SelectInfo &si);
    int getrfd() const
        { return rfd; }
    int getwfd() const
        { return wfd; }
    
    // noread() closes the rfd and makes this stream no longer valid for
    // reading.  nowrite() closes wfd and makes it no longer valid for
    // writing.
    void noread();
    void nowrite();
    
protected:
    int rfd, wfd;
    int in_progress;

    WvSplitStream(); // derived classes might not know the fds yet!
    
    virtual size_t uwrite(const void *buf, size_t size);
    virtual size_t uread(void *buf, size_t size);
};

#endif // __WVSPLITSTREAM_H
