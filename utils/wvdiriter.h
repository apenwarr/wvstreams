/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Directory iterator.  Recursively uses opendir and readdir, so you don't
 * have to.  Basically implements 'find'.
 *
 */

#ifndef __WVDIRITER_H
#define __WVDIRITER_H

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wvstring.h"

class WvDirIter
/*************/
{
public:
    WvDirIter( WvString _dirname );
    ~WvDirIter();

    bool isok() const;
    void rewind();
    bool next();

    const struct stat& operator () () const;
    WvString fname;     // updated by next()

private:
    WvString dirname;

    WvDirIter * child;

    struct dirent * dent;
    struct stat st;
    DIR * d;
};

#endif
