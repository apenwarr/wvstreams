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

struct WvDirEnt : public stat
/***************************/
{
    // we already have everything from struct stat, but we also want the
    // fullname, since it's useful
    WvString        fullname;
};

class WvDirIter
/*************/
{
public:
    WvDirIter( WvString _dirname, bool _recurse=true );
    ~WvDirIter();

    bool isok() const;
    void rewind();
    bool next();

    const WvDirEnt& operator () () const;
    const WvDirEnt * operator -> () const;

private:
    WvString    dirname;
    bool        recurse;

    WvDirIter * child;

    struct dirent * dent;
    DIR * d;

    WvDirEnt        info;

    void fill_info( WvString& fullname, struct stat& st );
};

#endif
