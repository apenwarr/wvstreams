/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvbackslash.h"
#include "wvbuf.h"
#include <wvstream.h>

static size_t wvtcl_escape(char *dst, WvStringParm s,
        const char *nasties, bool *verbatim = NULL)
{
    if (verbatim) *verbatim = false;

    // NULL strings remain such
    if (s.isnull())
        return 0;
    // empty strings are just {}
    if (!s)
    {
        if (dst)
        {
            dst[0] = '{';
            dst[1] = '}';
        }
	return 2;
    }
    
    WvString allnasties(WVTCL_ALWAYS_NASTY);
    allnasties.append(nasties);
    
    bool backslashify = false, inescape = false;
    int len = 0, unprintables = 0, bracecount = 0;
    const char *cptr;
    
    // figure out which method we need to use: backslashify or embrace.
    // also count the number of unprintable characters we'll need to 
    // backslashify, if it turns out that's necessary.
    for (cptr = s; *cptr; cptr++)
    {
        // Assume we do nothing
        if (dst) dst[len] = *cptr;
        ++len;

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
    {
        if (verbatim) *verbatim = true;
	return len; // no work needed!
    }
    
    if (backslashify)
    {
        if (dst)
        {
            len = 0;
            for (cptr = s; *cptr; ++cptr)
            {
                if (strchr(allnasties, *cptr)) dst[len++] = '\\';
                dst[len++] = *cptr;
            }
            return len;
        }
        else return len+unprintables;
    }
    else
    {
	// the embrace method: just take the string and put braces around it
        if (dst)
        {
            len = 0;
            dst[len++] = '{';
            for (cptr = s; *cptr; ++cptr)
                dst[len++] = *cptr;
            dst[len++] = '}';
            return len;
        }
        else return len+2;
    }
}


WvString wvtcl_escape(WvStringParm s, const char *nasties)
{
    bool verbatim;
    size_t len = wvtcl_escape(NULL, s, nasties, &verbatim);
    if (verbatim) return s;

    WvString result;
    result.setsize(len);
    char *e = result.edit();
    e += wvtcl_escape(e, s, nasties);
    *e = '\0';
    return result;
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
    int size = 0;

    WvList<WvString>::Iter i(l);
    for (i.rewind(); i.next(); )
        size += wvtcl_escape(NULL, *i, nasties);
    
    WvString result;
    result.setsize(size+1);

    char *p = result.edit();
    for (i.rewind(); i.next(); )
    {
        p += wvtcl_escape(p, *i, nasties);
        *p++ = ' ';
    }
    *--p = '\0';
    
    return result;
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
                incontinuation = true;
	    }
            else
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
		else if (ch == '}')
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
