/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A circular queue that can be accessed across fork().
 */
#include "wvmagiccircle.h"
#include <assert.h>


WvMagicCircle::WvMagicCircle(size_t _size)
    : shm(_size + 1 + 2*sizeof(int)), 
       head(((int*)shm.buf)[0]), tail(((int*)shm.buf)[1])
{
    assert((int)_size > 0);
    
    head = tail = 0;
    size = _size + 1; // a circular queue uses one extra byte
    circle = shm.cbuf + 2*sizeof(int);
    
    if (shm.geterr())
	seterr(shm);
}


WvMagicCircle::~WvMagicCircle()
{
    // nothing special
}


size_t WvMagicCircle::used()
{
    int x = tail - head;
    if (x < 0)
	x += size;
    assert(x >= 0);
    assert(x < size);
    
    return x;
}


size_t WvMagicCircle::put(const void *data, size_t len)
{
    size_t max = left();
    size_t chunk1;
    
    if (len > max)
	len = max;
    
    chunk1 = size - tail;
    if (len < chunk1)
	chunk1 = len;

#if 0
    WvLog log("put", WvLog::Info);
    log("put: head %s, tail %s, size %s, len %s, chunk1 %s\n",
	head, tail, size, len, chunk1);
#endif
    
    memcpy(circle + tail, data, chunk1);
    if (chunk1 < len)
	memcpy(circle, (char *)data + chunk1, len - chunk1);
    
    tail = (tail + len) % size;
    
    return len;
}


size_t WvMagicCircle::get(void *data, size_t len)
{
    size_t max = used();
    size_t chunk1;
    
    if (len > max)
	len = max;
    
    chunk1 = size - head;
    if (chunk1 > len)
	chunk1 = len;
    
    memcpy(data, circle + head, chunk1);
    if (chunk1 < len)
	memcpy((char *)data + chunk1, circle, len - chunk1);
    
    head = (head + len) % size;
    
    return len;
}


size_t WvMagicCircle::skip(size_t len)
{
    size_t max = used();
    
    if (len > max)
	len = max;
    
    head = (head + len) % size;
    
    return len;
}
