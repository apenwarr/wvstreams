/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 * 
 * Test for the 
 *
 */
#include "strutils.h"
#include <stdio.h>

int main()
{
    printf("Hostname: %s\n", hostname().cstr());
    printf("Fully Qualified Host Name: %s\n", fqdomainname().cstr());
    return 0;
}
