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
    bool result = loop.pre_select(si);
    
    loop.drain();
    
    if (!result)
    {
    	if (si.wants.readable)
    	{
	    if (circle.used())
	    	result = true;
    	}
        
    	if (si.wants.writable)
    	{
	    if (circle.left())
	    	result = true;
    	}
    }

    return result;
}  

size_t WvMagicLoopback::uread(void *buf, size_t len)
{
    size_t avail = circle.used();

    if (avail < len)
    	len = avail;
    
    return circle.get(buf, len);
}


size_t WvMagicLoopback::uwrite(const void *buf, size_t len)
{
    size_t howmuch = circle.left();
    
    if (len > howmuch)
	len = howmuch;
    
    len = circle.put(buf, len);
    
    if (len > 0)
        loop.uwrite("", 1); // Make select wake up

    return len;
}
