/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#include "wvfile.h"

#ifdef _WIN32
#include <io.h>
#define O_NONBLOCK 0
#define O_LARGEFILE 0
#define fcntl(a,b,c)
#endif

bool WvFile::open(WvStringParm filename, int mode, int create_mode)
{
    closed = stop_read = stop_write = false;
    
    errnum = 0;
    
    int xmode = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);

    skip_select = false;
    
    // don't do the default force_select of read if we're not readable!
    if (!readable)
	undo_force_select(true, false, false);
    
    close();
    int rwfd = ::open(filename, mode | O_NONBLOCK, create_mode);
    if (rwfd < 0)
    {
	seterr(errno);
	return false;
    }
    setfd(rwfd);
    fcntl(rwfd, F_SETFD, 1);
    return true;
}


// files not open for read are never readable; files not open for write
// are never writable.
bool WvFile::pre_select(SelectInfo &si)
{
    bool ret;
    
    SelectRequest oldwant = si.wants;
    
    if (!readable) si.wants.readable = false;
    if (!writable) si.wants.writable = false;
    ret = WvFDStream::pre_select(si);
    
    si.wants = oldwant;

    // Force select() to always return true by causing it to not wait and
    // setting our pre_select() return value to true.
    if (skip_select)
    {
	si.msec_timeout = 0;
	ret = true;
    }
    return ret;
}
