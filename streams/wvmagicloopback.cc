/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 */
 
#include "wvmagicloopback.h"

WvMagicLoopback::WvMagicLoopback(size_t size)
    : circle(size), loop()
{
}


bool WvMagicLoopback::pre_select(SelectInfo &si)
{
    loop.drain();

    loop.pre_select(si);

    if (si.wants.readable)
    {
        if (circle.used() > 0)
            return true;
    }
    
    if (si.wants.writable)
    {   
        if (circle.left() > 0)
            return true;
    }

    return false;
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
