/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple lockfile class using WvStreams.
 */

#include "wvlockfile.h"
#include <signal.h>

WvLockFile::WvLockFile(WvStringParm _lockname)
    : lockname(_lockname)
{ }


bool WvLockFile::isok()
{
    int pid = getpid();

    if ((pid > 0  && kill(pid, 0)) || !pid)
    {
        unlink(lockname);
        return true;
    }

    return false;
}


bool WvLockFile::lock(WvStringParm pid)
{
    if (!isok())
        return false;

    unlink(lockname);
    lockfile.open(lockname, O_WRONLY|O_CREAT|O_EXCL);

    if (!lockfile.isok())
    {
        lockfile.close();
        return false;
    }

    lockfile.print("%s\n", pid);
    lockfile.close();
    return true;
}


bool WvLockFile::unlock()
{
    unlink(lockname);

    if (!access(lockname, F_OK))
        return false;

    return true;
}


int WvLockFile::getpid()
{
    char* line;
    int pid;

    if (!access(lockname, W_OK))
        pid = -1;
    else
    {
        lockfile.open(lockname, O_RDONLY|O_CREAT);

        if (lockfile.isok())
        {
            // Get the pid from the file to make sure the process is still alive
            if ((line = lockfile.getline(0)) != NULL)
                pid = atoi(line);
            else
                pid = 0;
        }
        else
            pid = -1;

        lockfile.close();
    }
    return pid;
}
