/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Proper output:
 *  987.6 TB
 *  98.7 TB
 *  9.8 TB
 *  987.6 GB
 *  98.7 GB
 *  9.8 GB
 *  987.6 MB
 *  98.7 MB
 *  9.8 MB
 *  987.6 KB
 *  98.7 KB
 *  9.8 KB
 *  987 bytes
 *  98 bytes
 *  9 bytes
 *
 */

#include "strutils.h"
#include <stdio.h>

int main()
{
    long blocks = 987654321;
    long blocksize = 1000000;

    while( blocksize != 1 ) {
        printf( "%s\n", sizetoa( blocks, blocksize ).cstr() );
        blocksize /= 10;
    }

    while( blocks ) {
        printf( "%s\n", sizetoa( blocks, blocksize ).cstr() );
        blocks /= 10;
    }

    return( 0 );
}
