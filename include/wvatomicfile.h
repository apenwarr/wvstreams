/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * A simple class to access filesystem files using WvStreams.
 */
#ifndef __WVATOMFILE_H
#define __WVATOMFILE_H

#include "wvfile.h"

/**
 * WvAtomicFile implements a simple extension to wvfile to allow for 
 * atomic file creation.  Files normally can be created and written 
 * to, however, in the event of a kernel panic the file can be left in
 * an unusable state.
 *
 * A WvAtomicFile is atomically created on file close
 *
 */
class WvAtomicFile : public WvFile
{
private:
    WvString atomic_file, tmp_file;
    int tmpfd;
    bool atomic;

public:
    WvAtomicFile(int rwfd = -1);
    WvAtomicFile(WvStringParm filename, int mode, int create_mode = 0666);

    bool open(WvStringParm filename, int mode, int create_mode = 0666);
    void close();

    bool isatomic()
        { return atomic; }

};

#endif // __WVATOMFILE_H
