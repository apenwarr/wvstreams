/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple lockfile class using WvStreams.
 */

#include "wvlockfile.h"
#include "strutils.h"
#include <signal.h>


WvLockFile::WvLockFile(WvStringParm _lockname)
    : lockname(_lockname)
{
    // nothing special
}


bool WvLockFile::isok()
{
    pid_t pid = readpid();
    return !pid || pid == getpid();
}


bool WvLockFile::lock()
{
    if (!isok())
        return false;

    WvFile lock(lockname, O_WRONLY|O_CREAT|O_EXCL);
    if (!lock.isok())
        return false;

    lock.print("%s\n", getpid());
    return true;
}


bool WvLockFile::unlock()
{
    if (!isok())
	return false;

    unlink(lockname);

    return readpid() == 0;
}


int WvLockFile::readpid()
{
    char *line;
    pid_t pid = 0;
    WvString lockdir(getdirname(lockname));

    if (access(lockdir, W_OK) < 0 
      || (!access(lockname, F_OK) && access(lockname, R_OK) < 0))
	return -1; // won't be able to create a lock
    else
    {
        WvFile lock(lockname, O_RDONLY);
	line = lock.getline(-1);
	if (line)
	{
	    pid = atoi(line);
	    if (kill(pid, 0) < 0 && errno == ESRCH) // no such process
	    {
		// previous lock owner is dead; clean it up.
		::unlink(lockname);
		return 0;
	    }
	}
    }
    
    return pid;
}
