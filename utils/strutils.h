/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Various little utilities used primarily in the WvConfigFile class.
 *
 * Created:	Sept 12 1997	D. Coombs
 * Modified:	Nov  11 1997	D. Coombs
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
WvString hexdump_buffer( unsigned char *buf, size_t len );
extern bool isnewline( char c );
void hexify( char *obuf, unsigned char *ibuf, size_t len );
void unhexify( unsigned char *obuf, char *ibuf );


#endif // __STRUTILS_H
