/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A shared-memory zone via mmap().  If you create one of these, its 'buf'
 * element will be shared across fork() and you can use it for various
 * things (such as a circular queue, semaphore, etc).
 */
#ifndef __WVSHMZONE_H
#define __WVSHMZONE_H

#include "wverror.h"

class WvShmZone : public WvError
{
public:
    WvShmZone(size_t _size);
    ~WvShmZone();
    
private:
    int fd;
    
public:
    int size;
    
    union {
	void *buf;
	char *cbuf;
	unsigned char *ucbuf;
    };
};


#endif // __WVSHMZONE_h
