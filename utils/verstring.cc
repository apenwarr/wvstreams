/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Version number and string manipulations.  Version numbers are 32-bit
 * hexadecimal numbers such as 0x00012a00.  The first 16 bits are the major
 * version, and the second 16 bits are the (fractional) minor version.  For
 * example, the above example corresponds to version "1.2a" (which is the
 * version string).
 */
#include "verstring.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

const char *ver_to_string(unsigned int ver)
{
    static char str[10];
    unsigned int maj = (ver & 0xFFFF0000) >> 16, min = (ver & 0x0000FFFF);
    char *cptr;
    
    sprintf(str, "%x.%04x", maj, min);

    // trim off trailing zeroes from minor number
    for (cptr = strchr(str, 0); --cptr >= str; )
    {
	if (*cptr != '0')
	    break;
	
	if (cptr <= str  ||  *(cptr - 1) == '.')
	    break;
	
	*cptr = 0;
    }
    
    return str;
}


unsigned int string_to_ver(const char *str)
{
    static char lookup[] = "0123456789abcdef";
    unsigned int maj = 0, min = 0;
    unsigned char *cptr, *idx;
    int bits;
    
    // do the major number
    cptr = (unsigned char *)str;
    for (; *cptr && *cptr != '.' && *cptr != '_'; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	maj = (maj << 4) | ((char *)idx - lookup);
    }
    
    // do the minor number
    for (bits = 4; *cptr && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	min = (min << 4) | ((char *)idx - lookup);
	bits--;
    }
    
    return (maj << 16) | (min << (4*bits));
}
