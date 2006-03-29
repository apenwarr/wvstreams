/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A shared-memory zone via mmap().  See wvshmzone.h.
 */
#include "wvshmzone.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

WvShmZone::WvShmZone(size_t _size)
{
    size = (int)_size;
    assert(size > 0);
    
    buf = NULL;
    
    fd = open("/dev/zero", O_RDWR);
    if (fd < 0)
    {
	seterr(errno);
	return;
    }
    
    buf = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);              
    
    if (!buf)
    {
	seterr(errno);
	return;
    }
}


WvShmZone::~WvShmZone()
{
    if (buf)
	munmap(buf, size);
    if (fd >= 0)
	close(fd);
}

    
