/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvbuffer.h"


WvFastString wvtcl_escape(WvStringParm s, const char *nasties)
{
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
	
	if (*cptr=='{' || *cptr=='}' || *cptr=='"'
	  || strchr(nasties, *cptr))
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
	WvString out;
	out.setsize(strlen(s) + unprintables + 1);
	char *optr = out.edit();
	
	for (cptr = s; *cptr; cptr++)
	{
	    if (*cptr=='{' || *cptr=='}' || *cptr=='"'
	      || strchr(nasties, *cptr))
		*optr++ = '\\';
	    *optr++ = *cptr;
	}
	*optr = 0;
	
	return out;
    }
    else
    {
	// the embrace method: just take the string and put braces around it
	return WvString("{%s}", s);
    }
}


WvFastString wvtcl_unescape(WvStringParm s, bool do_embrace)
{
    // empty or NULL strings remain themselves
    if (!s)
	return s;
    
    int slen = strlen(s);
    
    // deal with embraced strings by simply removing the braces
    if (do_embrace && s[0] == '{' && s[slen-1] == '}')
    {
	WvString out;
	char *optr;
	
	out = s+1;
	optr = out.edit() + slen - 2;
	*optr = 0;
	return out;
    }
    
    // strings without backslashes don't need to be unbackslashified!
    if (!strchr(s, '\\'))
	return s;
    
    // otherwise, unbackslashify it.
    WvString out;
    out.setsize(slen);
    
    const char *cptr;
    char *optr = out.edit();
    for (cptr = s; *cptr; cptr++)
    {
	if (*cptr == '\\')
	{
	    cptr++;
	    if (!*cptr)
		break; // weird - ends in a backslash!
	    
	    // FIXME: support \x## and \### notation, perhaps?
	    switch (*cptr)
	    {
	    case 'n':
		*optr++ = '\n';
		break;
	    case 'r':
		*optr++ = '\r';
		break;
	    case 't':
		*optr++ = '\t';
		break;
	    case '0':
		*optr++ = 0;
		break;
	    default:
		*optr++ = *cptr;
		break;
	    }
	}
	else
	    *optr++ = *cptr;
    }
    *optr = 0;
    
    return out;
}


WvString wvtcl_encode(WvStringList &l, const char *nasties,
		      const char *splitchars)
{
    WvBuffer b;
    
    WvStringList::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	// elements are separated by spaces
	if (b.used())
	    b.put(splitchars, 1);
	
	// escape and add the element
	b.put(wvtcl_escape(*i));
    }
    
    return b.getstr();
}


void wvtcl_decode(WvStringList &l, WvStringParm _s,
		  const char *nasties, const char *splitchars)
{
    // empty or null strings are empty lists
    if (!_s)
	return;
    
    bool inescape = false, inquote = false;
    int bracecount = 0;
    WvString s(_s);
    char *sptr = s.edit(), *eptr;
    
    inquote = (*sptr == '"');
    
    for (eptr = sptr; *eptr; eptr++)
    {
	// end of a quoted/unquoted string
	if (!bracecount && !inescape 
	    && ((inquote && *eptr == '"')
		|| (!inquote && strchr(splitchars, *eptr))))
	{
	    *eptr = 0;
	    if (inquote)
		l.append(new WvString(wvtcl_unescape(sptr, false)), true);
	    else if (*sptr)
		l.append(new WvString(wvtcl_unescape(sptr, true)), true);
	    *eptr = 'X'; // make sure the loop doesn't exit!
	    sptr = eptr + 1;
	    
	    // if the next string begins with a quote, remember that.
	    inquote = (*sptr == '"');
	    sptr += inquote;
	    eptr += inquote;
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
    if (*sptr)
	l.append(new WvString(wvtcl_unescape(sptr)), true);
}


