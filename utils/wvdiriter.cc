/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Directory iterator.  Recursively uses opendir and readdir, so you don't
 * have to.  Basically implements 'find'.
 *
 */

#include "wvdiriter.h"

WvDirIter::WvDirIter( WvString _dirname, bool _recurse )
/******************************************************/
: dirname( _dirname )
{
    recurse = _recurse;

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

const WvDirEnt * WvDirIter::operator -> () const
/**********************************************/
{
    if( child )
        return( &(*child) () );
    else
        return( &info );
}

bool WvDirIter::next()
/********************/
// use readdir... and if that returns a directory, make a child and recurse
// into it.  If there's already a child, call ITS next() instead of ours.
{
    // recurse?
    if( recurse && !child && dent && S_ISDIR( info.st_mode ) )
        child = new WvDirIter( info.fullname );

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
        WvString fname;
        do {
            dent = readdir( d );
            if( dent ) {
                fname = WvString( "%s/%s", dirname, dent->d_name );
                ok = ( lstat( fname, &info ) == 0
                            && strcmp( dent->d_name, "." )
                            && strcmp( dent->d_name, ".." ) );
            }
        } while( dent && !ok );

        if( dent )
            info.fullname = fname;
        else {
            closedir( d );
            d = NULL;
        }

    } else
        dent = NULL;

    return( dent != NULL );
}

