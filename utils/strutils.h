/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 *
 * Various little string functions...
 *
 */

#ifndef __STRUTILS_H
#define __STRUTILS_H

#include "wvstring.h"

extern char *terminate_string( char *string, char c );
extern char *trim_string( char *string );
extern void replace_char( void *string, char c1, char c2, int length );
extern char *strlwr( char * string );
extern bool is_word( char * string );
extern WvString hexdump_buffer( unsigned char *buf, size_t len );
extern bool isnewline( char c );
extern void hexify( char *obuf, unsigned char *ibuf, size_t len );
extern void unhexify( unsigned char *obuf, char *ibuf );


#endif // __STRUTILS_H
