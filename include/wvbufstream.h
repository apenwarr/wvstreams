/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A buffered loopback stream.
 */ 
#ifndef __WVBUFSTREAM_H
#define __WVBUFSTREAM_H

#include "wvstream.h"

/**
 * WvBufStream stores data written by write(), and returns it
 * later on in read().
 * 
 * Handy for making virtual streams, like WvHttpPool.
 * 
 */
class WvBufStream : public WvStream
{
    bool dead, eof;
public:
    
    // when we close, set this pointer to NULL.  A bit hacky...
    WvStream **death_notify;
    
    WvBufStream();
    virtual ~WvBufStream();
	
    virtual void close();
    
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual bool isok() const;
    virtual bool pre_select(SelectInfo &si);
    
    void seteof() { eof = true; }
};


#endif // __WVBUFSTREAM_H
