/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 */
#ifndef __WVMAGICLOOPBACK_H
#define __WVMAGICLOOPBACK_H

#include "wvmagiccircle.h"
#include "wvloopback.h"
#include "wvlog.h"

class WvMagicLoopback : public WvStream
{
public:

    WvMagicLoopback(size_t size);
    
    /*** Overridden members ***/
    
    virtual bool pre_select(SelectInfo &si);
    
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);

private:
 
    WvMagicCircle circle;
    WvLoopback loop;
    WvLog log;
};

#endif // __WVMAGICLOOPBACK_H
