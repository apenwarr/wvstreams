/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple lockfile class using WvStreams.
 */

#ifndef __WVLOCKFILE_H
#define __WVLOCKFILE_H

#include "wvfile.h"


class WvLockFile
{
public:
    WvLockFile(WvStringParm _lockname);

    /**
     * Check to make sure no lock is established, and that we can acess
     * the lockfile.
     */
    bool isok();

    /**
     * Creates the lockfile with the given pid. Returns success/failure.
     */
    bool lock(WvStringParm pid);

    /**
     * Removes the lockfile if present. If there's no lockfile after,
     * returns true, otherwise false.
     */
    bool unlock();

    /**
     * Returns one of three things:
     *   => -1 if the lockfile exists, but is inaccessible.
     *   => 0 if there is no lockfile, or the process is not running.
     *   => The pid of the process if running and a lock is
     *      established and accessible.
     */
    int getpid();

protected:
    WvFile lockfile;
    WvString lockname;
};

#endif // __WVLOCKFILE_H
