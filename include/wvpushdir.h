/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvPushDir -- A simple class to check the existance of a dir 
 *  and to properly return the formatted path of the diir
 */
#ifndef __WVPUSHDIR_H
#define __WVPUSHDIR_H

#include "wverror.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <unistd.h>

class WvPushDir : public WvError
{
    DIR *dir_handle;
    char *old_dir;

public:
    // Prevent dynamic allocation: arbitrary sequence of creation/deletion
    // would just result in random directories getting popped in the wrong
    // sequence.  If you want to switch directories in a random order,
    // do it yourself with chdir().
    void* operator new(size_t) 
        { abort(); }

    WvPushDir(WvStringParm new_dir)
    {
        old_dir = new char[2048];
        if (!getcwd(old_dir, 2048)) {
            errnum = errno;
            return;
        }
       dir_handle = opendir(old_dir);
       if (chdir(new_dir) == -1)
          errnum = errno;
    }

    ~WvPushDir()
    { 
        chdir(old_dir); 
        closedir(dir_handle);
        free(old_dir);
    }
};

#endif /// __WVPUSHDIR_H
