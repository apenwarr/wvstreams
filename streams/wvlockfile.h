/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Some handy functions to create/remove /var/lock lockfiles.
 */
#ifndef __WVLOCKFILE_H
#define __WVLOCKFILE_H

#include "wvstring.h"

class WvLockFile
{
    WvString devicename, filename;
    int lock_count;
public:
    WvLockFile(WvString _devicename);
    ~WvLockFile();
    
    bool lock();
    void unlock();
    bool islocked()
       { return lock_count != 0; }
};

#endif // __WVLOCKFILE_H
