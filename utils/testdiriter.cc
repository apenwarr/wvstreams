/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Test program for WvDirIter.  Takes a directory on the command line, and
 * prints everything it sees.
 *
 */

#include "wvdiriter.h"

int main( int argc, char * argv[] )
/*********************************/
{
    if( argc < 2 ) {
        fprintf( stderr, "Specify a directory.\n" );
        return( -1 );
    }

    WvDirIter i( argv[1] );
    for( i.rewind(); i.next(); ) {
        printf( "%s -- mode %u -- size %lu\n", (const char *) i.fname,
                                               i().st_mode, i().st_size );
    }

    return( 0 );
}
