/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A simple class to access filesystem files using WvStreams.
 */
#ifndef __WVFILE_H
#define __WVFILE_H

#include "wvfdstream.h"
#include <fcntl.h>

/**
 * WvFile implements a stream connected to a file or Unix device.  We
 * include no support for operations like seek().  Since files are not
 * really streams, you probably do not need WvStream support for seekable
 * files; just use standard C I/O functions in that case.
 *
 * WvFile is primarily useful for Unix device files, which have defined
 * select() behaviour for example.
 */
class WvFile : public WvFDStream
{
public:
    WvFile(int rwfd = -1) : WvFDStream(rwfd)
    {
	if (rwfd > -1)
	{
	    /* We have to do it this way since O_RDONLY is defined as 0
	       in linux. */
	    mode_t xmode = fcntl (rwfd, F_GETFL);
	    xmode = xmode & (O_RDONLY | O_WRONLY | O_RDWR);
	    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
	    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);
	}
	else
	{
	    readable = writable = false;
	}
    }
    WvFile(WvStringParm filename, int mode, int create_mode = 0666)
        { open(filename, mode, create_mode); }
    bool open(WvStringParm filename, int mode, int create_mode = 0666);
    
    bool readable, writable;

    // Force select to always return true
    bool skip_select;
    
    virtual bool pre_select(SelectInfo &si);
};

#endif // __WVFILE_H
