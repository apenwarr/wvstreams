/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvFileWatcher class provides support for files which sometimes
 * have data appended at the end.  The WvStream::select() function is
 * overloaded to provide support for this as best we can (ie., polling
 * every 100ms or so).
 * 
 * See wvwatcher.h for more information.
 */
#include "wvwatcher.h"
#include <sys/time.h>
#include <errno.h>

WvFileWatcher::WvFileWatcher(const char *_filename, int _mode)
			: WvFile(_filename, _mode), filename(_filename)
{
    once_ok = WvFile::isok();
    openmode = _mode;
    fpos = 0;
    if (stat(filename.str, &last_st) && WvFile::isok())
    {
	errnum = errno;
	close();
	once_ok = false;
    }
}


bool WvFileWatcher::isok() const
{
    return once_ok;
}


bool WvFileWatcher::make_ok(bool retry)
{
    // if the inode has changed, or the size is smaller than last
    // time, or we cannot stat the file, close it and start over.
    struct stat st;
    if (fd >= 0 
	&& (stat(filename.str, &st)
	    || st.st_ino != last_st.st_ino || st.st_size < last_st.st_size))
    {
	close();
    }
    
    // make sure the file is open
    while (!WvFile::isok())
    {
	fpos = 0;
	if (!open(filename.str, openmode) && retry)
	    sleep(1);
	if (!retry && !WvFile::isok())
	    return false; // no such luck
    }

    stat(filename.str, &last_st);
    return true;
}


size_t WvFileWatcher::uread(void *buf, size_t size)
{
    size_t len;
    if (!once_ok) return 0; // this is never going to work
    select(-1, true, false, false);
    len = WvFile::uread(buf, size);
    fpos += len;
    return len;
}


size_t WvFileWatcher::uwrite(const void *buf, size_t size)
{
    size_t len;
    if (!once_ok) return 0;
    make_ok(true);
    len = WvFile::uwrite(buf, size);
    fpos += len;
    return len;
}


bool WvFileWatcher::select(time_t msec_timeout,
			   bool readable = true, bool writable = false,
			   bool isexception = false)
{
    struct stat st;
    
    if (!once_ok) return false;

    // if not waiting for r/w, then return true only if not waiting for
    // an exception either.
    if (!writable && !readable)
	return !isexception;
    
    struct timeval tv1, tv;
    time_t timeused;
    gettimeofday(&tv1, NULL);

    for (;;)
    {
	// readable if the current location pointer is < size
	if (fd >= 0 && readable && !fstat(fd, &st) && fpos < st.st_size)
	    return true;
	    
	// get the file open, at least
	if (make_ok(false))
	{
	    // writable as long as file is open
	    if (writable)
		return true;
	    
	    if (readable && fpos < last_st.st_size)
		return true;
	}
    
	gettimeofday(&tv, NULL);
	timeused = ((tv.tv_sec - tv1.tv_sec)*1000
		    + (tv.tv_usec - tv1.tv_usec)/1000);
	if (msec_timeout >= 0 && msec_timeout <= timeused)
	{
	    return false; // not ready, timed out
	}
	
	// wait for msec_timeout or 100ms, whichever is less
	if (msec_timeout >= 0)
	    usleep ((msec_timeout-timeused < 100 
		     ? msec_timeout-timeused : 100) * 1000);
	else
	    usleep(100 * 1000);
    }
    
    return false;
}


