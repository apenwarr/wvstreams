/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Deletes a section from a config file.  Useful when netmaps are too big.
 *
 */

#include "wvconf.h"

int main( int argc, char ** argv )
/********************************/
{
    if( argc != 3 ) {
        fprintf( stderr, "Usage: %s cfgfilename section\n", argv[0] );
        return( -1 );
    }

    WvConf cfg( argv[1] );
    cfg.delete_section( argv[2] );

    return( 0 );
}
