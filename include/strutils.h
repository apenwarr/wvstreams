/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little string functions...
 *
 */

#ifndef __STRUTILS_H
#define __STRUTILS_H

#if 0
// FIXME: this is needed on BSD and Win32
#include <time.h>
#endif

#include "wvstring.h"
#include "wvstringlist.h"

/**
 * Add character c to the end of a string after removing 
 * terminating carriage returns/linefeeds if any.
 * 
 * You need a buffer that's at least one character bigger than the 
 * current length of the string, including the terminating NULL. 
 */
char *terminate_string(char *string, char c);

/**
 * Trims whitespace from the beginning and end of the character string, 
 * including carriage return / linefeed characters. Modifies the string 
 * in place. Returns the new first character of the string, which points 
 * either at 'string' itself or some character contained therein.
 *
 * string is allowed to be NULL; returns NULL in that case.
 */
char *trim_string(char *string);

/**
 * Similar to above, but trims the string starting at the first occurrence of
 * c.
 */
char *trim_string(char *string, char c);

/**
 * Replaces all whitespace characters in the string with non-breaking spaces
 * (&nbsp;) for use with web stuff.
 */
char *non_breaking(char *string);

/**
 * Replace all instances of c1 with c2 for the first 'length' characters in 
 * 'string'. Ignores terminating NULL, so make sure you set 'length' correctly.
 */
void replace_char(void *string, char c1, char c2, int length);

/**
 * In-place modify a character string so that all contained letters are 
 * in lower case. Returns 'string'.
 */
char *strlwr(char *string);

/**
 * In-place modify a character string so that all contained letters are 
 * in upper case. Returns 'string'.
 */
char *strupr(char *string);

/**
 * Returns true if all characters in 'string' are isalnum() (alphanumeric).
 */
bool is_word(const char *string);

/**
 * Produce a hexadecimal dump of the data buffer in 'buf' of length 'len'. 
 * It is formatted with 16 bytes per line; each line has an address offset, 
 * hex representation, and printable representation.
 *
 * This is used mostly for debugging purposes. You can send the returned 
 * WvString object directly to a WvLog or any other WvStream for output.
 */
WvString hexdump_buffer(const void *buf, size_t len);

/**
 * Returns true if 'c' is a newline or carriage return character. 
 * Increases code readability a bit.
 */
bool isnewline(char c);

/**
 * Write the contents of the binary string of length 'len' pointed to by 'ibuf' 
 * into the output buffer 'obuf' in hexadecimal format.
 * 
 * For example, if len==4, ibuf=="ABCDEF", then obuf will contain "41424344"
 * with a terminating NULL character.
 * 
 * This is useful to turn arbitrary binary into a simple printable format, so
 * that it can (for example) be written to a WvConf configuration file.
 * 
 * obuf must be a buffer with at least (len * 2) + 1 bytes available. (two
 * digits for each byte of ibuf, plus a terminating NULL).
 */
void hexify(char *obuf, const void *ibuf, size_t len);

/**
 * Reverse the operation performed by hexify(). obuf must be a buffer large
 * enough to contain the entire binary output string; you can calculate this
 * size with (strlen(ibuf) / 2). obuf will NOT be automatically NULL-terminated.
 */
void unhexify(void *obuf, const char *ibuf);

/**
 * Converts escaped characters (things like %20 etc.) from web URLS into their
 * normal ASCII representations.
 */
WvString web_unescape(const char *str);

/**
 * Returns an RFC822-compatible date made out of _when, or, if _when < 0, out of
 * the current time.
 */
WvString rfc822_date(time_t _when = -1);

/**
 * Similar to crypt(), but this randomly selects its own salt.
 * This function is defined in strcrypt.cc.
 */
WvString passwd_crypt(const char *str);

/**
 * Returns a string with a backslash in front of every non alphanumeric
 * character in s1.
 */
WvString backslash_escape(WvStringParm s1);

/**
 * How many times does 'c' occur in "s"?
 */
int strcount(WvStringParm s, const char c);

/**
 * Example: encode_hostname_as_DN("www.fizzle.com")
 * will result in dc=www,dc=fizzle,dc=com,cn=www.fizzle.com
 */
WvString encode_hostname_as_DN(WvStringParm hostname);

/**
 * Given a hostname, turn it into a "nice" one.  It has to start with a
 * letter/number, END with a letter/number, have underscores converted to
 * hyphens, and have no more than one hyphen in a row.  If we can't do this
 * and have any sort of answer, return "UNKNOWN".
 */
WvString nice_hostname(WvStringParm name);

/**
 * Seperates the filename and directory name within a path.
 */
WvString getfilename(WvStringParm fullname);
WvString getdirname(WvStringParm fullname);

#endif // __STRUTILS_H
