/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little string functions...
 *
 */


#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <sys/types.h> // off_t
#include <time.h>
#include "wvstring.h"
#include "wvstringlist.h"
#include "wvhex.h"

#ifdef _WIN32
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/** \file
 * Various little string functions
 */


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
 * Snip off the first part of 'haystack' if it consists of 'needle'.
 */
char *snip_string(char *haystack, char *needle);

#ifndef _WIN32
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

#endif

/** Returns true if all characters in 'string' are isalnum() (alphanumeric). */
bool is_word(const char *string);

/**
 * Produce a hexadecimal dump of the data buffer in 'buf' of length 'len'. 
 * It is formatted with 16 bytes per line; each line has an address offset, 
 * hex representation, and printable representation.
 *
 * This is used mostly for debugging purposes. You can send the returned 
 * WvString object directly to a WvLog or any other WvStream for output.
 */
WvString hexdump_buffer(const void *buf, size_t len, bool charRep = true);

/**
 * Returns true if 'c' is a newline or carriage return character. 
 * Increases code readability a bit.
 */
bool isnewline(char c);

/**
 * Converts escaped characters (things like %20 etc.) from web URLS into their
 * normal ASCII representations.
 */
WvString web_unescape(const char *str);


/**
 * Converts all those pesky spaces, colons, and other nasties into nice unreadable
 * Quasi-Unicode codes
 */
WvString url_encode(WvStringParm stuff);
 

/**
 * Returns an RFC822-compatible date made out of _when, or, if _when < 0, out of
 * the current time.
 */
WvString rfc822_date(time_t _when = -1);

/** Returns an RFC1123-compatible date made out of _when */
WvString rfc1123_date(time_t _when);

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

/** How many times does 'c' occur in "s"? */
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
 * Take a full path/file name and splits it up into respective pathname and
 * filename. This can also be useful for splitting the toplevel directory off a
 * path.
 */
WvString getfilename(WvStringParm fullname);
WvString getdirname(WvStringParm fullname);

/**
 * Given a number of blocks and a blocksize (default==1 byte), return a 
 * WvString containing a human-readable representation of blocks*blocksize.
 */
WvString sizetoa(long long blocks, int blocksize=1);

/**
 * Finds a string in an array and returns its index.
 * Returns -1 if not found.
 */
int lookup(const char *str, const char * const *table,
    bool case_sensitive = false);

/**
 * Splits a string and adds each substring to a collection.
 *   coll       : the collection of strings to add to
 *   _s         : the string to split
 *   splitchars : the set of delimiter characters
 *   limit      : the maximum number of elements to split
 */
template<class StringCollection>
void strcoll_split(StringCollection &coll, WvStringParm _s,
    const char *splitchars = " \t", int limit = 0)
{
    WvString s(_s);
    char *sptr = s.edit(), *eptr, oldc;

    while (sptr && *sptr)
    {
	--limit;
	if (limit)
	{
	    sptr += strspn(sptr, splitchars);
	    eptr = sptr + strcspn(sptr, splitchars);
	}
	else
	{
	    sptr += strspn(sptr, splitchars);
	    eptr = sptr + strlen(sptr);
	}
	
	oldc = *eptr;
	*eptr = 0;
	
	WvString *newstr = new WvString(sptr);
        coll.add(newstr, true);
	
	*eptr = oldc;
	sptr = eptr;
    }
}

/**
 * Splits a string and adds each substring to a collection.
 *   this behaves differently in that it actually delimits the 
 *   pieces as fields and returns them, it doesn't treat multiple
 *   delimeters as one and skip them.
 *
 *   ie., parm1::parm2 -> 'parm1','','parm2' when delimited with ':'
 *
 *   coll       : the collection of strings to add to
 *   _s         : the string to split
 *   splitchars : the set of delimiter characters
 *   limit      : the maximum number of elements to split
 */
template<class StringCollection>
void strcoll_splitstrict(StringCollection &coll, WvStringParm _s,
    const char *splitchars = " \t", int limit = 0)
{
    WvString s(_s);
    char *cur = s.edit();

    for (;;)
    {
        --limit;
        if (!limit)
        {
            coll.add(new WvString(cur), true);
            break;
        }

        int len = strcspn(cur, splitchars);

        char tmp = cur[len];
        cur[len] = 0;
        coll.add(new WvString(cur), true);
        cur[len] = tmp;

        if (!cur[len]) break;
        cur += len + 1;
    }
}

/**
 * Concatenates all strings in a collection and returns the result.
 *   coll      : the collection of strings to read from
 *   joinchars : the delimiter string to insert between strings
 */
template<class StringCollection>
WvString strcoll_join(const StringCollection &coll,
    const char *joinchars = " \t")
{
    size_t joinlen = strlen(joinchars);
    size_t totlen = 1;
    typename StringCollection::Iter s(
        const_cast<StringCollection&>(coll));
    for (s.rewind(); s.next(); )
    {
        if (s->cstr())
            totlen += strlen(s->cstr());
        totlen += joinlen;
    }
    totlen -= joinlen; // no join chars at tail
    
    WvString total;
    total.setsize(totlen);

    char *te = total.edit();
    te[0] = 0;
    bool first = true;
    for (s.rewind(); s.next(); )
    {
        if (first)
            first = false;
        else
            strcat(te, joinchars);
        if (s->cstr()) 
            strcat(te, s->cstr());
    }
    return total;
}

/**
 * Replace any instances of "a" with "b" in "s".  Kind of like sed, only
 * much dumber.
 */
WvString strreplace(WvStringParm s, WvStringParm a, WvStringParm b);

/**
 * Replace any consecutive instances of character c with a single one
 */
WvString undupe(WvStringParm s, char c);

WvString hostname();

WvString fqdomainname();

/**
 * Inserts SI-style spacing into a number
 * (eg passing 9876543210 returns "9 876 543 210")
 */
WvString metriculate(const off_t i);

#endif // __STRUTILS_H
