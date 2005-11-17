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
#include "wvverstring.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

const char *old_ver_to_string(unsigned int ver)
{
    static char str[10];
    unsigned int maj = (ver & 0xFFFF0000) >> 16, min = (ver & 0x0000FFFF);

    sprintf(str, "%x.%04x", maj, min);
    trim_verstr(str);
    
    return str;
}


const char *new_ver_to_string(unsigned int ver)
{
    static char str[11];
    unsigned int maj = (ver & 0xFF000000) >> 24, min = (ver & 0x00FF0000) >> 16,
                 rev = (ver & 0x0000FFFF);

    sprintf(str, "%x.%02x.%04x", maj, min, rev);

    return str;
}


const char *ver_to_string(unsigned int ver)
{
    if (is_new_ver(ver))
        return new_ver_to_string(ver);

    return old_ver_to_string(ver);
}


unsigned int string_to_old_ver(const char *str)
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


unsigned int string_to_new_ver(const char *str)
{
    static char lookup[] = "0123456789abcdef";
    unsigned int maj = 0, min = 0, rev = 0, ver;
    unsigned char *cptr, *idx;
    int bits;

    // do the major number
    cptr = (unsigned char *)str;
    for (; *cptr; cptr++)
    {
	if (*cptr == '.' || *cptr == '_')
	{
	    cptr++;
	    break;
	}
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	maj = (maj << 4) | ((char *)idx - lookup);
    }
    
    // do the minor number
    for (bits = 2; *cptr && *cptr != '.' && *cptr != '_' && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	min = (min << 4) | ((char *)idx - lookup);
	bits--;
    }
    
    // do the revision number
    for (bits = 4; *cptr && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;

	rev = (rev << 4) | ((char *)idx - lookup);
	bits--;
    }

    ver = (maj << 24) | (min << 16) | (rev << (4*bits));

    return ver;
}


unsigned int string_to_ver(const char *str)
{
    if (is_new_verstr(str))
        return string_to_new_ver(str);

    return string_to_old_ver(str);
}


bool is_new_ver(unsigned int ver)
{
    return (ver & 0xff000000);
}


bool is_new_verstr(const char *str)
{
    char *p = strchr(str, '.');
    if (p && strchr(p+1, '.'))
        return true;

    return false;
}


char *trim_verstr(char *verstr)
{
    // trim off trailing zeroes
    char *cptr;

    for (cptr = strchr(verstr, 0); --cptr >= verstr; )
    {
	if (*cptr != '0')
	    break;
	
	if (cptr <= verstr  ||  *(cptr - 1) == '.')
	    break;
	
	*cptr = 0;
    }
    return verstr;
}
