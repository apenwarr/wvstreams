/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * Regular expression support though libc
 */ 
#ifndef __WVREGEX_H
#define __WVREGEX_H

#include "wverror.h"
#include "wvstring.h"
#include <sys/types.h>
#include <regex.h>

#define __WVRE_REG(n) __wvre_r##n
#define __WVRE_DECL_FORM(n) WvString &__WVRE_REG(n) = WvRegex::__wvre_null_reg
#define WVREGEX_REGS_DECL \
    	    	__WVRE_DECL_FORM( 0), __WVRE_DECL_FORM( 1), \
    	    	__WVRE_DECL_FORM( 2), __WVRE_DECL_FORM( 3), \
		__WVRE_DECL_FORM( 4), __WVRE_DECL_FORM( 5), \
		__WVRE_DECL_FORM( 6), __WVRE_DECL_FORM( 7), \
		__WVRE_DECL_FORM( 8), __WVRE_DECL_FORM( 9), \
		__WVRE_DECL_FORM(10), __WVRE_DECL_FORM(11), \
		__WVRE_DECL_FORM(12), __WVRE_DECL_FORM(13), \
		__WVRE_DECL_FORM(14), __WVRE_DECL_FORM(15), \
		__WVRE_DECL_FORM(16), __WVRE_DECL_FORM(17), \
		__WVRE_DECL_FORM(18), __WVRE_DECL_FORM(19)
#define __WVRE_CALL_FORM(n) __WVRE_REG(n)
#define WVREGEX_REGS_CALL \
    	    	__WVRE_CALL_FORM( 0), __WVRE_CALL_FORM( 1), \
    	    	__WVRE_CALL_FORM( 2), __WVRE_CALL_FORM( 3), \
		__WVRE_CALL_FORM( 4), __WVRE_CALL_FORM( 5), \
		__WVRE_CALL_FORM( 6), __WVRE_CALL_FORM( 7), \
		__WVRE_CALL_FORM( 8), __WVRE_CALL_FORM( 9), \
		__WVRE_CALL_FORM(10), __WVRE_CALL_FORM(11), \
		__WVRE_CALL_FORM(12), __WVRE_CALL_FORM(13), \
		__WVRE_CALL_FORM(14), __WVRE_CALL_FORM(15), \
		__WVRE_CALL_FORM(16), __WVRE_CALL_FORM(17), \
		__WVRE_CALL_FORM(18), __WVRE_CALL_FORM(19)

/**
 * Unified support for regular expressions
 */
class WvRegex: public WvErrorBase
{
private:
    bool have_preg;
    regex_t preg;
    
    bool match(WvStringParm string, int eflags,
    	    size_t nmatch, regmatch_t pmatch[]) const;

    virtual void seterr(int _errnum);

public:
    // cflags: flags that affect interpretation of the regex
    enum {
    	// Use (obsolete) basic regex syntax (like grep).  See regex(7).
    	BASIC = 0,
    	// Use extended regex syntax (like egrep).  See regex(7).
    	EXTENDED = REG_EXTENDED,
    	// Case insensitive
    	ICASE = REG_ICASE,
    	// Do not collect match start and end or registers; faster
    	NOSUB = REG_NOSUB,
    	// Match-any-character operators don't match a newline.  See regex(3)
    	NEWLINE = REG_NEWLINE
    };
    static const int default_cflags;

    // eflags: flags that affect matching of regex
    enum
    {
    	// Matching begining of line always fails (unless NEWLINE cflag is set)
    	NOTBOL = REG_NOTBOL,
    	// Matching end of line always fails (unless NEWLINE cflag is set)
    	NOTEOL = REG_NOTEOL
    };
    static const int default_eflags;

    // Internal use only
    static WvString __wvre_null_reg;

    // Construct an empty regex object.  Matches will always fail until set()
    // is called with a valid regex.
    WvRegex() : have_preg(false) {}
    // Construct a regex object, compiling the given regex
    WvRegex(WvStringParm regex, int cflags = default_cflags) : have_preg(false)
    	{ set(regex, cflags); }
    ~WvRegex();
    
    // Replace the current regex to match with a new one.
    bool set(WvStringParm regex, int cflags = default_cflags);
    
    // The match functions match a given string against the compiled regex.
    //
    // All match functions return return true if a match was found and false
    // if not.
    //
    // If eflags are given, they affect the matching as described above.
    //
    // If a match was found and match_start and match_end are given,
    // they will contain the offset in the string of the start of the first 
    // match and the end of the first match PLUS ONE.
    //
    // If a match was found and registers are given they will contain the
    // substrings of the match that correspond to the parenthesized
    // expressions in the regex.
    //
    // Examples:
    // int match_start, match_end;
    // WvString reg1, reg2, reg3;
    // re.match("a string");
    // re.match("a string", match_start, match_end);
    // re.match("a string", reg1);
    // re.match("a string", reg1, reg2, reg3);
    // re.match("a string", WvRegex::NOTBOL);
    // re.match("a string", WvRegex::NOTEOL,
    //      match_start, match_end, reg1, reg2, reg3);
    //
    bool match(WvStringParm string, int eflags,
    	    int &match_start, int &match_end, WVREGEX_REGS_DECL) const
    {
    	regmatch_t pmatch[21];
    	int nmatch = 1;

#define __WVRE_COUNT_REGS(n) \
    	if ( &__WVRE_REG(n) != &__wvre_null_reg) ++nmatch

    	__WVRE_COUNT_REGS( 0); __WVRE_COUNT_REGS( 1);
    	__WVRE_COUNT_REGS( 2); __WVRE_COUNT_REGS( 3);
    	__WVRE_COUNT_REGS( 4); __WVRE_COUNT_REGS( 5);
    	__WVRE_COUNT_REGS( 6); __WVRE_COUNT_REGS( 7);
    	__WVRE_COUNT_REGS( 8); __WVRE_COUNT_REGS( 9);
    	__WVRE_COUNT_REGS(10); __WVRE_COUNT_REGS(11);
    	__WVRE_COUNT_REGS(12); __WVRE_COUNT_REGS(13);
    	__WVRE_COUNT_REGS(14); __WVRE_COUNT_REGS(15);
    	__WVRE_COUNT_REGS(16); __WVRE_COUNT_REGS(17);
    	__WVRE_COUNT_REGS(18); __WVRE_COUNT_REGS(19);

     	if (!match(string, eflags, nmatch, pmatch)) return false;

     	match_start = pmatch[0].rm_so;
     	match_end = pmatch[0].rm_eo;

#define __WVRE_STORE_REGS(n) \
     	if (&__WVRE_REG(n) != &__wvre_null_reg \
     	    	&& pmatch[n+1].rm_so != -1 && pmatch[n+1].rm_eo != -1) \
     	{ \
     	    int len = pmatch[n+1].rm_eo-pmatch[n+1].rm_so; \
     	    __WVRE_REG(n).setsize(len+1); \
     	    memcpy(__WVRE_REG(n).edit(), &string[pmatch[n+1].rm_so], len); \
     	    __WVRE_REG(n).edit()[len] = '\0'; \
     	}

    	__WVRE_STORE_REGS( 0); __WVRE_STORE_REGS( 1);
    	__WVRE_STORE_REGS( 2); __WVRE_STORE_REGS( 3);
    	__WVRE_STORE_REGS( 4); __WVRE_STORE_REGS( 5);
    	__WVRE_STORE_REGS( 6); __WVRE_STORE_REGS( 7);
    	__WVRE_STORE_REGS( 8); __WVRE_STORE_REGS( 9);
    	__WVRE_STORE_REGS(10); __WVRE_STORE_REGS(11);
    	__WVRE_STORE_REGS(12); __WVRE_STORE_REGS(13);
    	__WVRE_STORE_REGS(14); __WVRE_STORE_REGS(15);
    	__WVRE_STORE_REGS(16); __WVRE_STORE_REGS(17);
    	__WVRE_STORE_REGS(18); __WVRE_STORE_REGS(19);
     	     	
     	return true;
    }
    bool match(WvStringParm string, WVREGEX_REGS_DECL) const
    {
    	int match_start, match_end;
    	return match(string, default_eflags,
    	    	match_start, match_end, WVREGEX_REGS_CALL); 
    }
    bool match(WvStringParm string, int eflags, WVREGEX_REGS_DECL) const
    {
    	int match_start, match_end;
    	return match(string, eflags,
    	    	match_start, match_end, WVREGEX_REGS_CALL); 
    }
    bool match(WvStringParm string, int &match_start, int &match_end,
    	    WVREGEX_REGS_DECL) const
    {
    	return match(string, default_eflags,
    	    	match_start, match_end, WVREGEX_REGS_CALL); 
    }
};

#endif // __WVREGEX_H
