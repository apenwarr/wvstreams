/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Takes a string on the command line and attempts to turn it into a
 * hexadecimal version number.
 *
 * Mainly useful, stupidly enough, for the softupdate database.
 */

#include "verstring.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned int ver = 0;
    if (argc == 2)
        ver = string_to_ver(argv[1]);

    printf("0x%08x\n", ver);
    return 0;
}
