/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvbackslash.h"
#include "wvbuffer.h"
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


WvString wvtcl_encode(WvStringList &l, const char *nasties,
		      const char *splitchars)
{
    WvDynamicBuffer b;
    WvStringList::Iter i(l);
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

WvString wvtcl_getword(WvBuffer &buf, const char *splitchars, bool do_unescape)
{
    int origsize = buf.used();
    if (origsize == 0)
        return WvString::null;

    bool inescape = false, inquote = false;
    int bracecount = 0;
    char *sptr = (char *)buf.get(origsize);
    char *eptr = sptr;

    inquote = (*sptr == '"');
    
    // If the first character is a quote, we need to move
    // eptr so we don't get just an empty 0 character string.
    if (inquote)
        eptr++;
    
    for (; *eptr && (eptr - sptr < origsize); eptr++)
    {
	// end of a quoted/unquoted string
	if (!bracecount && !inescape 
	    && ((inquote && *eptr == '"')
		|| (!inquote && strchr(splitchars, *eptr))))
	{
	    if (inquote)
		eptr++;
          
	    char olde = *eptr;
	    *eptr = 0;
            WvString toreturn;  // NULL string
	    if (*sptr)
	    {
                // This new stuff here was added to deal with a memory 
                // corruption issue.
                char newstring[(eptr - sptr)+1];
                strncpy(newstring, sptr, (eptr - sptr));
                newstring[(eptr - sptr)] = '\0';
		if (do_unescape)
		    toreturn = wvtcl_unescape(newstring);
		else
		    toreturn = newstring;
	    }
	    *eptr = olde; // make sure the loop doesn't exit!
	    if (inquote)
		eptr--;
            origsize -= (eptr - sptr) + 1;
            if (! toreturn.isnull())
            {
                buf.unget(origsize);
                return toreturn;
            }
            else
            {
                sptr = eptr + 1;
                // if the next string begins with a quote, remember that.
                inquote = (*sptr == '"');
                if (inquote)
                    eptr++;
            }
	    continue;
	}
	
	if (!inquote && !inescape && *eptr == '{')
	    bracecount++;
	else if (!inquote && !inescape && *eptr == '}')
	    bracecount--;
	
	if (*eptr == '\\')
	    inescape = !inescape;
	else
	    inescape = false;
    }
    
    // finished the string - get the terminating element, if any.
    if (*sptr && !bracecount && !inquote)
    {
        // This new stuff here was added due to a memory corruption issue.
        char newstring[(eptr - sptr)+1];
        strncpy(newstring, sptr, (eptr - sptr));
        newstring[(eptr - sptr)] = '\0';
        if (do_unescape)
            return wvtcl_unescape(newstring);
        else
            return newstring;
    }
    else
    {
        buf.unget(origsize);
    }
   return WvString::null;
}

void wvtcl_decode(WvStringList &l, WvStringParm _s,
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
