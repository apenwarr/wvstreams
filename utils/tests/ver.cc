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
#include <string.h>

int main(int argc, char *argv[])
{
    unsigned int ver = 0;
    if (argc == 2)
    {
        // if the given string doesn't have any dots, assume it's a
        // new-style version filename, and insert them where they ought to
        // go.
        char buf[20];
        if (!strchr(argv[1], '.') && !strchr(argv[1], '_'))
        {
            int len = strlen(argv[1]);
            memset(buf, '0', 10);
            strcpy(buf+10-len, argv[1]);
            memmove(buf, buf+2, 2);
            buf[2]='.';
            memmove(buf+3, buf+4, 2);
            buf[5]='.';
        }
        else
            strncpy(buf, argv[1], 19);

        ver = string_to_ver(buf);
    }

    printf("0x%08x\n", ver);
    return 0;
}
