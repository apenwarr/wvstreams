/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * Implementation of regular expression support though libc
 */
#include "wvregex.h"

WvString WvRegex::__wvre_null_reg;

const int WvRegex::default_cflags = WvRegex::EXTENDED;
const int WvRegex::default_eflags = 0;
    
void WvRegex::seterr(int errcode)
{
    int error_desc_len = ::regerror(errcode, &preg, NULL, 0);
    if (error_desc_len > 0)
    {
        WvString error_desc;
        error_desc.setsize(error_desc_len);
        ::regerror(errcode, &preg, error_desc.edit(), error_desc_len);
        WvErrorBase::seterr_both(errcode, error_desc);
    }
    else WvErrorBase::seterr(errcode);
}

bool WvRegex::set(WvStringParm regex, int cflags)
{
    if (have_preg) ::regfree(&preg);

    int errcode = ::regcomp(&preg, regex, cflags);
    if (errcode)
    {
        seterr(errcode);
        have_preg = false;
    }
    else have_preg = true;
    
    return have_preg;
}

WvRegex::~WvRegex()
{
    if (have_preg) ::regfree(&preg);
}

bool WvRegex::match(WvStringParm string, int eflags,
    	size_t nmatch, regmatch_t pmatch[]) const
{
    if (!have_preg) return false;

    return ::regexec(&preg, string, nmatch, pmatch, eflags) == 0;
}
