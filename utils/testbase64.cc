/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Test program for base64 functions...
 */

#include "base64.h"
#include <stdio.h>

int main()
/********/
{
    char * str = "Aladdin:open sesame";
    char * enc = base64_encode( str );
    printf( "%s\n%s\n%s\n", str, enc, base64_decode( enc ) );
    return( 0 );
}
