/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little string functions...
 *
 */

#ifndef __STRUTILS_H
#define __STRUTILS_H

#include "wvstring.h"

/**
 * Add character c to the end of a string after removing 
 * terminating carriage returns/linefeeds if any.
 * 
 * You need a buffer that's at least one character bigger than the 
 * current length of the string, including the terminating NULL. 
 */
extern char *terminate_string(char *string, char c);

/**
 * Trims whitespace from the beginning and end of the character string, 
 * including carriage return / linefeed characters. Modifies the string 
 * in place. Returns the new first character of the string, which points 
 * either at 'string' itself or some character contained therein.
 *
 * string is allowed to be NULL; returns NULL in that case.
 */
extern char *trim_string(char *string);

/**
 * Similar to above, but trims the string starting at the first occurrence of
 * c.
 */
extern char *trim_string(char *string, char c);

/**
 * Replace all instances of c1 with c2 for the first 'length' characters in 
 * 'string'. Ignores terminating NUL, so make sure you set 'length' correctly.
 */
extern void replace_char(void *string, char c1, char c2, int length);

/**
 * In-place modify a character string so that all contained letters are 
 * in lower case. Returns 'string'.
 */
extern char *strlwr(char *string);

/**
 * In-place modify a character string so that all contained letters are 
 * in upper case. Returns 'string'.
 */
extern char *strupr(char *string);

/**
 * Returns true if all characters in 'string' are isalnum() (alphanumeric).
 */
extern bool is_word(const char *string);

/**
 * Produce a hexadecimal dump of the data buffer in 'buf' of length 'len'. 
 * It is formatted with 16 bytes per line; each line has an address offset, 
 * hex representation, and printable representation.
 *
 * This is used mostly for debugging purposes. You can send the returned 
 * WvString object directly to a WvLog or any other WvStream for output.
 */
extern WvString hexdump_buffer(const void *buf, size_t len);

/**
 * Returns true if 'c' is a newline or carriage return character. 
 * Increases code readability a bit.
 */
extern bool isnewline(char c);

/**
 * Write the contents of the binary string of length 'len' pointed to by 'ibuf' 
 * into the output buffer 'obuf' in hexadecimal format.
 * 
 * For example, if len==4, ibuf=="ABCDEF", then obuf will contain "41424344" with 
 * a terminating NUL character.
 * 
 * This is useful to turn arbitrary binary into a simple printable format, so that 
 * it can (for example) be written to a WvConf configuration file.
 * 
 * obuf must be a buffer with at least (len * 2) + 1 bytes available. (two digits 
 * for each byte of ibuf, plus a terminating NUL).
 */
extern void hexify(char *obuf, const void *ibuf, size_t len);

/**
 * Reverse the operation performed by hexify(). obuf must be a buffer large enough 
 * to contain the entire binary output string; you can calculate this size with 
 * (strlen(ibuf) / 2). obuf will NOT be automatically NUL-terminated.
 */
extern void unhexify(void *obuf, const char *ibuf);

/**
 * Converts escaped characters (things like %20 etc.) from web URLS into their
 * normal ASCII representations.
 */
extern WvString web_unescape(const char *str);

WvString rfc822_date(time_t when = -1);
WvString passwd_crypt(const char *str);
WvString backslash_escape(const WvString &s1);

extern int strcount(const WvString &s, const char c);

#endif // __STRUTILS_H
