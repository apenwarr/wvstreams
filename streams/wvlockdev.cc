/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Some handy functions to create/remove /var/lock lockfiles.
 */
#include "wvlockdev.h"
#include "wvfile.h"
#include "strutils.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

WvLockDev::WvLockDev(WvString _devicename)
	: devicename(_devicename)
{
    const char *p = strrchr(devicename, '/');
    if (p)
	p++;
    else
	p = devicename;
    
    lock_count = 0;
    filename = WvString("/var/lock/LCK..%s", p);
}


WvLockDev::~WvLockDev()
{
    if (lock_count)
    {
	lock_count = 1;
	unlock();
    }
}


#if USE_LOCKDEV /* use the liblockdev.a locking routines */

#include <lockdev.h>

bool WvLockDev::lock()
{
    if (lock_count)
    {
	lock_count++;
	return true;
    }
    
    if (dev_lock(devicename))
	return false;

    lock_count++;
    return true;
}


void WvLockDev::unlock()
{
    if (!lock_count) return;

    if (!--lock_count)
	dev_unlock(devicename, getpid());
}


#else /* !USE_LOCKDEV -- implement our own locking routines */


// note: this function uses the O_EXCL flag to open(), and thus assumes
// that /var/lock is not an NFS-mounted drive (according to the open() man
// page, you need to follow a special procedure to ensure successful NFS
// locking)
//
// Actually there may be other race conditions that we should look into.
bool WvLockDev::lock()
{
    pid_t pid;
    
    if (lock_count)
    {
	lock_count++;
    	return true;
    }

    WvFile fd(filename, O_RDWR | O_EXCL | O_CREAT, 0644);

    if (fd.isok()) 
    {
	// We made a lock file...
	fd.print("%10s\n", getpid());
    }
    else if (fd.geterr() == EEXIST)
    {
	char *inbuf;
	
    	// Lock file is already there!  Check for staleness...
    	sleep(1);	// preventing race condition...
 	
	fd.open(filename, O_RDONLY);
	//fprintf(stderr, "ok: %d\n", fd.isok());
	inbuf = trim_string(fd.getline(-1));
	//fprintf(stderr, "getline: '%s'\n", inbuf);
	
	if (inbuf)
	    pid = atoi(inbuf);
	else
	    pid = 0;
	
	//fprintf(stderr, "pid: '%d'\n", pid);
	
 	if (pid && pid != -1 && kill(pid, 0) == -1 && errno == ESRCH)
	{
 	    // we can create a lockfile now
	    fd.close();
 	    if (unlink(filename))
		return false; // cannot remove lockfile
 	    fd.open(filename, O_RDWR | O_EXCL | O_CREAT, 0644);
	    fd.print("%10s\n", getpid());
 	}
	else
 	    return false; // device already locked
    }
    else // some other unexpected error
	return false;

    lock_count++;
    return true;
}



void WvLockDev::unlock()
{
    if (!lock_count) return;

    if (!--lock_count)
	unlink(filename);
}


#endif /* !USE_LOCKDEV */
