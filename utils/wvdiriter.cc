/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Directory iterator.  Recursively uses opendir and readdir, so you don't
 * have to.  Basically implements 'find'.
 *
 */

#include "wvdiriter.h"

WvDirIter::WvDirIter( WvString dirname, bool _recurse )
/*****************************************************/
: dir( dirs )
{
    recurse = _recurse;
    go_up   = false;
    info.relpath = WvString("");

    DIR * d = opendir( dirname );
    if( d ) {
        Dir * dd = new Dir( d, dirname );
        dirs.prepend( dd, true );
    }
}

WvDirIter::~WvDirIter()
/*********************/
{
    dirs.zap();
}

bool WvDirIter::isok() const
/**************************/
{
    return( dirs.count() > 0 );
}

void WvDirIter::rewind()
/**********************/
{
    // have to closedir() everything that isn't the one we started with,
    // and rewind that.
    while( dirs.count() > 1 ) {
        dir.rewind();
        dir.next();
        dir.unlink();
    }

    if( isok() ) {
        dir.rewind();
        dir.next();
        rewinddir( dir->d );
    }
}


bool WvDirIter::next()
/********************/
// use readdir... and if that returns a directory, opendir() it and prepend
// it to dirs, so we start reading it until it's done.
{
    struct dirent * dent = NULL;

    if( !isok() )
        return( false );

    bool tryagain;
    do {
        bool ok = false;
        tryagain = false;

        // unrecurse if the user wants to
        if( go_up ) {
            go_up = false;
            if( dirs.count() > 1 ) {
                dir.unlink();
                dir.rewind();
                dir.next();
            } else
                return( false );
        }

        do {
            dent = readdir( dir->d );
            if( dent ) {
                info.fullname = WvString( "%s/%s", dir->dirname, dent->d_name );
                info.name = dent->d_name;
                ok = ( lstat( info.fullname, &info ) == 0
                            && strcmp( dent->d_name, "." )
                            && strcmp( dent->d_name, ".." ) );
            }
        } while( dent && !ok );

        if( dent ) {
            // recurse?
            if( recurse && S_ISDIR( info.st_mode ) ) {
                DIR * d = opendir( info.fullname );
                if( d ) {
                    info.relpath = WvString("%s%s", info.relpath, info.name);
                    Dir * dd = new Dir( d, info.fullname );
                    dirs.prepend( dd, true );
                    dir.rewind();
                    dir.next();
                }
            }
        } else {
            // end of directory.  if we recursed, unlink it and go up a 
            // notch.  if this is the top level, DON'T close it, so that
            // the user can ::rewind() again if he wants.
            if( dirs.count() > 1 ) {
                if (dirs.count() == 2)
                    info.relpath = WvString("");
                else
                    info.relpath = getdirname(info.relpath);

                dir.unlink();
                dir.rewind();
                dir.next();
                tryagain = true;
            }
        }

    } while( tryagain );

    return( dent != NULL );
}

