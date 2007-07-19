/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 */
 
#include "wvmagicloopback.h"

WvMagicLoopback::WvMagicLoopback(size_t size)
    : circle(size), loop()
{
}


void WvMagicLoopback::pre_select(SelectInfo &si)
{
    loop.drain();

    loop.pre_select(si);

    if ((si.wants.readable && circle.used() > 0) ||
        (si.wants.writable && circle.left() > 0))
        si.msec_timeout = 0;
}  


bool WvMagicLoopback::post_select(SelectInfo &si)
{
    bool ret = WvStream::post_select(si);

    if ((si.wants.readable && circle.used() > 0) ||
        (si.wants.writable && circle.left() > 0))
        ret = true;

    return ret;
}


size_t WvMagicLoopback::uread(void *buf, size_t len)
{
    return circle.get(buf, len);
}


size_t WvMagicLoopback::uwrite(const void *buf, size_t len)
{
    len = circle.put(buf, len);
    
    if (len > 0)
        loop.uwrite("", 1); // Make select wake up

    return len;
}
