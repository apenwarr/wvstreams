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
: fname( "" ), dirname( _dirname )
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

const struct stat& WvDirIter::operator () () const
/************************************************/
// if we have a child, call ITS operator ()... otherwise use st.
{
    if( child )
        return( (*child) () );
    else
        return( st );
}

bool WvDirIter::next()
/********************/
// use readdir... and if that returns a directory, make a child and recurse
// into it.  If there's already a child, call ITS next() instead of ours.
{
    if( child ) {
        if( child->next() ) {
            fname = child->fname;
            return true;
        } else {
            delete child;
            child = NULL;   // fall through!
        }
    }

    if( d ) {
        bool ok = false;
        do {
            dent = readdir( d );
            if( dent ) {
                fname = WvString( "%s/%s", dirname, dent->d_name );
                ok = ( lstat( fname, &st ) == 0
                            && strcmp( dent->d_name, "." )
                            && strcmp( dent->d_name, ".." ) );
            }
        } while( dent && !ok );

        // recurse next time?
        if( dent && S_ISDIR( st.st_mode ) )
            child = new WvDirIter( fname );

    } else
        dent = NULL;

    if( dent == NULL )
        fname = "";

    return( dent != NULL );
}

