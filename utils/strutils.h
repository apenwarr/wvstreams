/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Various little string functions...
 *
 */

#ifndef __STRUTILS_H
#define __STRUTILS_H

#include "wvstring.h"

char *terminate_string(char *string, char c);
char *trim_string(char *string);
void replace_char(void *string, char c1, char c2, int length);
char *strlwr(char * string);
bool is_word(char * string);
WvString hexdump_buffer(unsigned char *buf, size_t len);
bool isnewline(char c);
void hexify(char *obuf, unsigned char *ibuf, size_t len);
void unhexify(unsigned char *obuf, char *ibuf);
WvString web_unescape(const char *str);
WvString rfc822_date(time_t when=-1);
WvString passwd_crypt(const char *str);


#endif // __STRUTILS_H
