/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for WvDirIter.  Takes a directory on the command line, and
 * prints everything it sees.
 *
 */

#include "wvdiriter.h"

int main( int argc, char * argv[] )
/*********************************/
{
    WvString dirname;
    bool     recurse = false;
    bool     skip_mounts = false;

    for( int i=1; i<argc; i++ ) {
        if( !strcmp( argv[i], "-R" ) )
            recurse = true;
        else if ( !strcmp( argv[i], "-S" ) )
            skip_mounts = true;
        else
            dirname = argv[i];
    }

    if( !dirname ) {
        fprintf( stderr, "Usage: %s [-R] [-S] directory\n", argv[0] );
        return( -1 );
    }

    WvDirIter i( dirname, recurse, skip_mounts );
    for( i.rewind(); i.next(); ) {
        printf( "%s -- mode %u -- size %lu\n", (const char *) i->fullname,
                                               i->st_mode, i->st_size );
    }

    return( 0 );
}
