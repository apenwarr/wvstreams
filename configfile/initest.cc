
/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * The test program for the WvConfigFile class.  Not to be used in real life.
 *
 * Created:	Sept 12 1997		D. Coombs
 * Modified:	Sept 21 1997		D. Coombs
 * 
 */
#include <stdio.h>
#include "wvconf.h"

int main( int argc, char * argv[] )
{
    WvConf cfg("testfile.ini");
    char * test_str;

    if( !cfg.isok() ) {
    	return( -1 );
    }

    if( argc != 3 ) {
    	printf( "Usage: %s section name\n", argv[0] );
    	return( -1 );
    }

    test_str = cfg.get( argv[1], argv[2], "not there" );
    printf( "%d\n", cfg.get( "intl", "icountry", 5 ) );
    printf( "%s\n\n", test_str );
    if( !cfg.isok() ) {
    	return( -1 );
    }

    cfg.set( "Windows", "FooPort", "testywesty\n\n\n        \n" );
    if( !cfg.isok() ) {
    	return( -1 );
    }

    WvConfigSection * 	sect = cfg.get_section( "fontsubSTitUtES" );
    if( sect ) {
    	sect->dump( stdout );
    }
    puts( "" );
    sect = cfg.get_section( "should_not_exist" );

    return( 0 );
}
