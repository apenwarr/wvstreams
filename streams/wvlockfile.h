/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Some handy functions to create/remove /var/lock lockfiles.
 */
#ifndef __WVLOCKFILE_H
#define __WVLOCKFILE_H

#include "wvstring.h"

/**
 * Class to handle Lock files - usefull for WvDial, and other places where we 
 * need to guarantee exclusive access to a file or device. Creates/Removes lockfiles
 * in /var/lock
 */
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
