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

struct WvDirEnt
/*************/
{
    // copied directly from struct stat
    dev_t           dev;
    ino_t           ino;
    mode_t          mode;
    nlink_t         nlink;
    uid_t           uid;
    gid_t           gid;
    dev_t           rdev;
    off_t           size;
    unsigned long   blksize;
    unsigned long   blocks;
    time_t          atime;
    time_t          mtime;
    time_t          ctime;

    // and, since it's useful
    WvString        fullname;
};

class WvDirIter
/*************/
{
public:
    WvDirIter( WvString _dirname );
    ~WvDirIter();

    bool isok() const;
    void rewind();
    bool next();

    const WvDirEnt& operator () () const;

private:
    WvString dirname;

    WvDirIter * child;

    struct dirent * dent;
    DIR * d;

    WvDirEnt        info;

    void fill_info( WvString& fullname, struct stat& st );
};

#endif
