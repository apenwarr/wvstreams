/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * Version number and string manipulations.  Version numbers are 32-bit
 * hexadecimal numbers such as 0x00012a00.  The first 16 bits are the major
 * version, and the second 16 bits are the (fractional) minor version.  For
 * example, the above example corresponds to version "1.2a" (which is the
 * version string).
 */
#include "verstring.h"
#include <stdio.h>

int main()
{
    printf("%s %08x\n", ver_to_string(0x99998888), string_to_ver("1.0"));
    printf("%s %08x\n", ver_to_string(0x01a00200), string_to_ver(".02a"));
    printf("%s %08x\n", ver_to_string(0x00001000), string_to_ver("1b"));
    printf("%s %08x\n", ver_to_string(0x00000000), string_to_ver("1A."));
    
    return 0;
}
