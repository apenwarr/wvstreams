/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvFileWatcher class provides support for files which sometimes
 * have data appended at the end.  It only polls as often as your select()
 * delay, so be careful!
 * 
 * The file is rewound and reopened if its inode changes or its
 * length gets shorter, under the assumption that we will want to see the
 * entire contents of the new file.
 *
 * See wvwatcher.h.
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
    if (stat(filename, &last_st) && WvFile::isok())
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
	&& (stat(filename, &st)
	    || st.st_ino != last_st.st_ino || st.st_size < last_st.st_size))
    {
	close();
    }
    
    // make sure the file is open
    while (!WvFile::isok())
    {
	fpos = 0;
	if (!open(filename, openmode) && retry)
	    sleep(1);
	if (!retry && !WvFile::isok())
	    return false; // no such luck
    }

    stat(filename, &last_st);
    return true;
}


size_t WvFileWatcher::uread(void *buf, size_t size)
{
    size_t len;
    if (!once_ok) return 0; // this is never going to work
    
    if (select(0))
    {
	len = WvFile::uread(buf, size);
	fpos += len;
	return len;
    }
    
    return 0;
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


// since you cannot really select() on a real file, we fake it:
// select_setup() returns true if the file is longer than it was last time,
// or you want to write to it.  Otherwise the file is "not ready."
// Because this happens in select_setup, the file will only be checked as
// often as your select() delay.  So infinite delays are a bad idea!
bool WvFileWatcher::select_setup(SelectInfo &si)
{
    struct stat st;
    
    if (!once_ok) return false;
    
    if (si.writable)
	return true; // always writable
    
    if (!si.readable)
	return false; // what are you asking?
    
    // readable if the current location pointer is < size
    if (fd >= 0 && si.readable && !fstat(fd, &st) && fpos < st.st_size)
	return true;
	    
    // get the file open, at least
    if (make_ok(false))
    {
	// writable as long as file is open
	if (si.writable)
	    return true;
	
	if (si.readable && fpos < last_st.st_size)
	    return true;
    }
    
    return false;
}
