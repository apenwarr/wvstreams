/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvbackslash.h"
#include "wvbuf.h"
#include <wvstream.h>

static size_t wvtcl_escape(char *dst, const char *s, size_t s_len,
        const char *nasties, bool *verbatim = NULL)
{
    if (verbatim) *verbatim = false;

    // NULL strings remain such
    if (s == NULL)
        return 0;
    // empty strings are just {}
    if (s_len == 0)
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
    const char *cptr, *cptr_end = s + s_len;
    
    // figure out which method we need to use: backslashify or embrace.
    // also count the number of unprintable characters we'll need to 
    // backslashify, if it turns out that's necessary.
    for (cptr = s; cptr != cptr_end; cptr++)
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
            for (cptr = s; cptr != cptr_end; ++cptr)
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
            for (cptr = s; cptr != cptr_end; ++cptr)
                dst[len++] = *cptr;
            dst[len++] = '}';
            return len;
        }
        else return len+2;
    }
}


WvString wvtcl_escape(WvStringParm s, const char *nasties)
{
    size_t s_len = s.len();

    bool verbatim;
    size_t len = wvtcl_escape(NULL, s, s_len, nasties, &verbatim);
    if (verbatim) return s;

    WvString result;
    result.setsize(len);
    char *e = result.edit();
    e += wvtcl_escape(e, s, s_len, nasties);
    *e = '\0';
    return result;
}


static size_t wvtcl_unescape(char *dst, const char *s, size_t s_len,
        bool *verbatim = NULL)
{
    //printf("  unescape '%s'\n", (const char *)s);
    
    // empty or NULL strings remain themselves
    if (!s)
    {
        if (verbatim) *verbatim = true;
	return 0;
    }

    if (verbatim) *verbatim = false;
    
    // deal with embraced strings by simply removing the braces
    if (s[0] == '{' && s[s_len-1] == '}')
    {
        if (dst) memcpy(dst, &s[1], s_len-2);
        return s_len - 2;
    }
    
    bool skipquotes = false;
    // deal with quoted strings by ignoring the quotes _and_ unbackslashifying.
    if (s[0] == '"' && s[s_len-1] == '"')
	skipquotes = true;
    
    // otherwise, unbackslashify it.
    const char *start = s, *end = &s[s_len];
    if (skipquotes)
    {
        ++start;
        --end;
    }
    size_t len = 0;
    for (; start != end; ++start)
    {
        if (*start != '\\')
        {
            if (dst) dst[len] = *start;
            len++;
        }
    }
    return len;
}


WvString wvtcl_unescape(WvStringParm s)
{
    size_t s_len = s.len();

    bool verbatim;
    size_t len = wvtcl_unescape(NULL, s, s_len, &verbatim);
    if (verbatim) return s;

    WvString result;
    result.setsize(len+1);
    char *e = result.edit();
    e += wvtcl_unescape(e, s, s_len);
    *e = '\0';
    return result;
}


WvString wvtcl_encode(WvList<WvString> &l, const char *nasties,
		      const char *splitchars)
{
    int size = 0;

    WvList<WvString>::Iter i(l);
    for (i.rewind(); ; )
    {
        size += wvtcl_escape(NULL, *i, i->len(), nasties);
        if (i.next())
            ++size;
        else break;
    }
    
    WvString result;
    result.setsize(size+1);

    char *p = result.edit();
    for (i.rewind(); ; )
    {
        p += wvtcl_escape(p, *i, i->len(), nasties);
        if (i.next())
            *p++ = splitchars[0];
        else break;
    }
    *p = '\0';
    
    return result;
}

const size_t WVTCL_GETWORD_NONE (UINT_MAX);

static size_t wvtcl_getword(char *dst, const char *s, size_t s_len,
        const char *splitchars, bool do_unescape, size_t *end = NULL)
{
    //printf("      used=%d\n", origsize);
    if (!s_len) return WVTCL_GETWORD_NONE;

    bool inescape = false, inquote = false, incontinuation = false;
    int bracecount = 0;
    const char *origend = s + s_len;
    const char *sptr, *eptr;

    // skip leading separators
    for (sptr = s; sptr != origend; sptr++)
    {
	if (!strchr(splitchars, *sptr))
	    break;
    }

    if (sptr == origend) // nothing left
        return WVTCL_GETWORD_NONE;

    // detect initial quote
    if (*sptr == '"')
    {
        inquote = true;
	eptr = sptr+1;
    }
    else
	eptr = sptr;
    
    // loop over string until something satisfactory is found
    for (; eptr != origend; eptr++)
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
	// not there yet...
        return WVTCL_GETWORD_NONE;

    //printf("len=%d, unget=%d\n", eptr - sptr, origend - eptr);
    if (end) *end = eptr - s;

    if (do_unescape)
        return wvtcl_unescape(dst, sptr, eptr-sptr);
    else
    {
        if (dst) memcpy(dst, sptr, eptr-sptr);
        return eptr - sptr;
    }
}


WvString wvtcl_getword(WvBuf &buf, const char *splitchars, bool do_unescape)
{
    int origsize = buf.used();
    const char *origptr = (const char *)buf.get(origsize);

    size_t end;
    size_t len = wvtcl_getword(NULL, origptr, origsize,
            splitchars, do_unescape, &end);
    if (len == WVTCL_GETWORD_NONE)
    {
        buf.unget(origsize);
        return WvString::null;
    }

    WvString result;
    result.setsize(len+1);
    char *e = result.edit();
    e += wvtcl_getword(e, origptr, origsize, splitchars, do_unescape);
    *e = '\0';

    buf.unget(origsize - end);

    return result;
}


void wvtcl_decode(WvList<WvString> &l, WvStringParm _s,
		  const char *splitchars, bool do_unescape)
{
    const char *s = _s;
    size_t s_len = _s.len();
    for (;;)
    {
        size_t end;
        size_t len = wvtcl_getword(NULL, s, s_len,
                splitchars, do_unescape, &end);
        if (len == WVTCL_GETWORD_NONE)
            break;

        WvString *word = new WvString();
        word->setsize(len+1);

        char *e = word->edit();
        e += wvtcl_getword(e, s, s_len, splitchars, do_unescape);
        *e = '\0';
        l.append(word, true);

        s += end;
        s_len -= end;
    }
}
