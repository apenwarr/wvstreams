/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Directory iterator.  Recursively uses opendir and readdir, so you don't
 * have to.  Basically implements 'find'.
 *
 */

#include "wvdiriter.h"

WvDirIter::WvDirIter( WvString _dirname )
/***************************************/
: dirname( _dirname )
{
    dent = NULL;
    child = NULL;

    d = opendir( dirname );
}

WvDirIter::~WvDirIter()
/*********************/
{
    if( child ) {
        delete child;
        child = NULL;
    }

    if( d ) {
        closedir( d );
        d = NULL;
    }
}

bool WvDirIter::isok() const
/**************************/
{
    return( d != NULL );
}

void WvDirIter::rewind()
/**********************/
{
    if( d )
        rewinddir( d );
}

const WvDirEnt& WvDirIter::operator () () const
/*********************************************/
// if we have a child, call ITS operator ()... otherwise use 'info'.
{
    if( child )
        return( (*child) () );
    else
        return( info );
}

bool WvDirIter::next()
/********************/
// use readdir... and if that returns a directory, make a child and recurse
// into it.  If there's already a child, call ITS next() instead of ours.
{
    if( child ) {
        if( child->next() ) {
            return true;
        } else {
            delete child;
            child = NULL;   // fall through!
        }
    }

    if( d ) {
        bool ok = false;
        struct stat st;
        WvString fname;
        do {
            dent = readdir( d );
            if( dent ) {
                fname = WvString( "%s/%s", dirname, dent->d_name );
                ok = ( lstat( fname, &st ) == 0
                            && strcmp( dent->d_name, "." )
                            && strcmp( dent->d_name, ".." ) );
            }
        } while( dent && !ok );

        if( dent ) {
            fill_info( fname, st );

            // recurse next time?
            if( S_ISDIR( info.mode ) )
                child = new WvDirIter( info.fullname );
        } else {
            closedir( d );
            d = NULL;
        }

    } else
        dent = NULL;

    return( dent != NULL );
}

void WvDirIter::fill_info( WvString& fullname, struct stat& st )
/**************************************************************/
{
    info.dev        = st.st_dev;
    info.ino        = st.st_ino;
    info.mode       = st.st_mode;
    info.nlink      = st.st_nlink;
    info.uid        = st.st_uid;
    info.gid        = st.st_gid;
    info.rdev       = st.st_rdev;
    info.size       = st.st_size;
    info.blksize    = st.st_blksize;
    info.blocks     = st.st_blocks;
    info.atime      = st.st_atime;
    info.mtime      = st.st_mtime;
    info.ctime      = st.st_ctime;

    info.fullname   = fullname;
}
