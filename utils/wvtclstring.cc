/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvbackslash.h"
#include "wvbuf.h"
#include <wvstream.h>

WvString wvtcl_escape(WvStringParm s, const char *nasties)
{
    WvString allnasties(WVTCL_ALWAYS_NASTY);
    allnasties.append(nasties);
    
    bool backslashify = false, inescape = false;
    int unprintables = 0, bracecount = 0;
    const char *cptr;
    
    // NULL strings remain such
    if (!(const char *)s)
	return s;
    
    // empty strings are just {}
    if (!s)
	return "{}";
    
    // figure out which method we need to use: backslashify or embrace.
    // also count the number of unprintable characters we'll need to 
    // backslashify, if it turns out that's necessary.
    for (cptr = s; *cptr; cptr++)
    {
	if (!inescape && *cptr == '{')
	    bracecount++;
	else if (!inescape && *cptr == '}')
	    bracecount--;
	if (bracecount < 0)
	    backslashify = true;
	
	if (strchr(allnasties.cstr(), *cptr))
	    unprintables++;

	if (*cptr == '\\')
	    inescape = !inescape;
	else
	    inescape = false;
    }
    
    if (!backslashify && !unprintables)
	return s; // no work needed!
    
    if (backslashify)
    {
	// the backslashify method: backslash-escape _all_ suspicious chars.
        return WvBackslashEncoder(allnasties).strflushstr(s, true);
    }
    else
    {
	// the embrace method: just take the string and put braces around it
	return WvString("{%s}", s);
    }
}


WvString wvtcl_unescape(WvStringParm s)
{
    //printf("  unescape '%s'\n", (const char *)s);
    
    // empty or NULL strings remain themselves
    if (!s)
	return s;
    
    int slen = s.len();
    bool skipquotes = false;
    
    // deal with embraced strings by simply removing the braces
    if (s[0] == '{' && s[slen-1] == '}')
    {
	WvString out;
	char *optr;
	
	out = s+1;
	optr = out.edit() + slen - 2;
	*optr = 0;
	return out;
    }
    
    // deal with quoted strings by ignoring the quotes _and_ unbackslashifying.
    if (s[0] == '"' && s[slen-1] == '"')
	skipquotes = true;
    
    // strings without backslashes don't need to be unbackslashified!
    if (!skipquotes && !strchr(s, '\\'))
	return s;
    
    // otherwise, unbackslashify it.
    return WvBackslashDecoder().strflushmem(
        s.cstr() + int(skipquotes),
        slen - int(skipquotes) * 2, true);
}


WvString wvtcl_encode(WvList<WvString> &l, const char *nasties,
		      const char *splitchars)
{
    WvDynBuf b;
    WvList<WvString>::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	// elements are separated by spaces
	if (b.used())
	    b.put(splitchars, 1);
	
	// escape and add the element
	b.putstr(wvtcl_escape(*i, nasties));
    }
    
    return b.getstr();
}

WvString wvtcl_getword(WvBuf &buf, const char *splitchars, bool do_unescape)
{
    int origsize = buf.used();
    if (origsize == 0)
        return WvString::null;

    bool inescape = false;
    bool inquote = false;
    bool incontinuation = false;
    int bracecount = 0;
    const char *sptr = (const char*)buf.get(origsize);
    int len = 0;

    // detect initial quote
    if (*sptr == '"')
    {
        inquote = true;
        len = 1;
    }
    
    // loop over string until something satisfactory is found
    for (; len < origsize; ++len)
    {
        char ch = sptr[len];
        
        // handle escapes and line continuations
        // we wait until we receive the next character after these
        // before returning any data
        incontinuation = false;
        if (inescape)
        {
            inescape = false;
            if (ch == '\n')
            {
                incontinuation = true;
                continue; // need a character from the next line
            }
        }
        else
        {
            if (ch == '\\')
            {
                inescape = true;
                continue; // need a character to complete escape
            }
        }

        // detect end of a quoted/unquoted string
        if (bracecount == 0)
        {
            if (inquote)
            {
                if (ch == '"')
                {
                    len += 1; // include the quote
                    goto returnstring;
                }
            }
            else
            {
                if (strchr(splitchars, ch))
                {
                    origsize -= 1; // consume the delimiter
                    goto returnstring;
                }
            }
        }
        // match braces
        if (! inquote)
        {
            if (ch == '{')
                bracecount += 1;
            else if (ch == '}')
                bracecount -= 1;
        }
    }
    
    // finished the string - get the terminating element, if any.
    if (bracecount == 0 && !inquote && !inescape && !incontinuation)
        goto returnstring;

    // give up
    buf.unget(origsize);
    return WvString::null;

returnstring:
    char str[len + 1];
    memcpy(str, sptr, len);
    str[len] = '\0';
    buf.unget(origsize - len);

    if (do_unescape)
        return wvtcl_unescape(str).unique();
    return WvString(str).unique();
}

void wvtcl_decode(WvList<WvString> &l, WvStringParm _s,
		  const char *splitchars, bool do_unescape)
{
    // empty or null strings are empty lists
    if (!_s)
	return;

    WvConstStringBuffer buf(_s);
    while (buf.used() > 0)
    {
        WvString appendword = wvtcl_getword(buf, splitchars, do_unescape);
        if (! appendword.isnull())
        {
            l.append(new WvString(appendword), true);
        }
        else
            break; 
    }
}
