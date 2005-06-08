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
    
    // if the braces aren't balanced, backslashify
    if (bracecount != 0)
        backslashify = true;

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
    //printf("      used=%d\n", origsize);
    if (!origsize) return WvString();

    bool inescape = false, inquote = false, incontinuation = false;
    int bracecount = 0;
    const char *origptr = (const char *)buf.get(origsize), 
	       *origend = origptr + origsize;
    const char *sptr = origptr, *eptr;

    // skip leading separators
    for (sptr = origptr; sptr < origend; sptr++)
    {
	if (!strchr(splitchars, *sptr))
	    break;
    }

    if (sptr >= origend) // nothing left
    {
        buf.unget(origsize);
	//printf("ungot %d\n", origsize);
        return WvString();
    }

    // detect initial quote
    if (*sptr == '"')
    {
        inquote = true;
	eptr = sptr+1;
    }
    else
	eptr = sptr;
    
    // loop over string until something satisfactory is found
    for (; (eptr-origptr) < origsize; eptr++)
    {
	char ch = *eptr;
	
        incontinuation = false;
	
        if (inescape)
        {
            if (ch == '\n')
	    {
		// technically we've finished the line-continuation
		// sequence, but we require at least one more character
		// in order to prove that there's a next line somewhere
		// in the buffer.  Otherwise we might stop parsing before
		// we're "really" done if we're given input line-by-line.
		// 
		// A better way to do this would be for getword() to *never*
		// return a string unless it contains a separator character;
		// then we wouldn't need this weird special case.  But it
		// don't work like that; we'll return the last word in the
		// buffer even if it *doesn't* end in a separator character.
                incontinuation = true;
	    }
	    inescape = false;
        }
        else if (ch == '\\')
	{
	    inescape = true;
	    // now we need a character to complete the escape
        }
	else // not an escape sequence
	{
	    // detect end of a quoted/unquoted string
	    if (bracecount == 0)
	    {
		if (inquote)
		{
		    if (ch == '"')
		    {
			eptr++;
			break;
		    }
		}
		else if (strchr(splitchars, ch))
		    break;
	    }
	    
	    // match braces
	    if (!inquote)
	    {
		if (ch == '{')
		    bracecount++;
		else if (bracecount > 0 && ch == '}')
		    bracecount--;
	    }
	}
    }
    
    if (bracecount || sptr==eptr || inquote || inescape || incontinuation)
    {
	// not there yet...
	buf.unget(origsize);
	return WvString();
    }

    WvString ret;
    ret.setsize(eptr - sptr + 1);
    char *retptr = ret.edit();
    memcpy(retptr, sptr, eptr-sptr);
    retptr[eptr-sptr] = 0;
    
    //printf("len=%d, unget=%d\n", eptr - sptr, origend - eptr);
    buf.unget(origend - eptr);

    if (do_unescape)
        return wvtcl_unescape(ret);
    else
	return ret;
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
        if (appendword.isnull())
	    break;
	
	l.append(new WvString(appendword), true);
    }
}
