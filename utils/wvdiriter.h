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
#include "wvlinklist.h"

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
    WvDirIter( WvString dirname, bool _recurse=true );
    ~WvDirIter();

    bool isok() const;
    void rewind();
    bool next();

    const WvDirEnt& operator () () const;
    const WvDirEnt * operator -> () const;

private:
    bool        recurse;

    WvDirEnt        info;

    struct Dir {
        Dir( DIR * _d, WvString _dirname )
            : d( _d ), dirname( _dirname )
            {}
        ~Dir()
            { if( d ) closedir( d ); }

        DIR *    d;
        WvString dirname;
    };

    DeclareWvList( Dir );
    DirList       dirs;
    DirList::Iter dir;
};

#endif
