/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of a simple and efficient printable-string class.  Most
 * of the class is actually inlined and can be found in wvstring.h.
 */
#include "wvstring.h"
#include <ctype.h>
#include <assert.h>

WvStringBuf WvFastString::nullbuf = { 0, 1 };
const WvFastString WvFastString::null;


// always a handy function
static inline int _max(int x, int y)
{
    return x>y ? x : y;
}


void WvFastString::setsize(size_t i)
{
    unlink();
    newbuf(i);
}



WvFastString::WvFastString()
{
    link(&nullbuf, NULL);
}


WvFastString::WvFastString(const WvFastString &s)
{
    link(s.buf, s.str);
}


WvFastString::WvFastString(const WvString &s)
{
    link(s.buf, s.str);
}


WvFastString::WvFastString(const char *_str)
{
    // just copy the pointer - no need to allocate memory!
    str = (char *)_str; // I promise not to change anything!
    buf = NULL;
}


void WvString::copy_constructor(const WvFastString &s)
{
    if (!s.buf)
    {
	link(&nullbuf, s.str);
	unique();
    }
    else
	link(s.buf, s.str); // already in a nice, safe WvStreamBuf
}


WvString::WvString(const char *_str)
{
    link(&nullbuf, _str);
    
    // apenwarr (2002/04/24): from now on, all WvString objects are created
    // with unique(), so you should _never_ have to call it explicitly.  We
    // still can (and should!) use fast parameter passing via WvFastString.
    unique();
}


// NOTE: make sure that 32 bytes is big enough for your longest int.
// This is true up to at least 64 bits.
WvFastString::WvFastString(int i) // auto-render int 'i' into a string
{
    newbuf(32);
    sprintf(str, "%d", i);
}


WvFastString::~WvFastString()
{
    unlink();
}


void WvFastString::unlink()
{ 
    if (buf && ! --buf->links)
	free(buf);
}
    

void WvFastString::link(WvStringBuf *_buf, const char *_str)
{
    buf = _buf;
    if (buf)
	buf->links++;
    str = (char *)_str; // I promise not to change it without asking!
}
    

WvStringBuf *WvFastString::alloc(size_t size)
{ 
    WvStringBuf *abuf = (WvStringBuf *)malloc(WVSTRINGBUF_SIZE(buf)
					     + size + WVSTRING_EXTRA);
    abuf->links = 0;
    abuf->size = size;
    return abuf;
}


void WvString::append(WvStringParm s)
{
    if (s)
    {
	if (*this)
	    *this = WvString("%s%s", *this, s);
	else
	    *this = s;
    }
}


size_t WvFastString::len() const
{
    return str ? strlen(str) : 0;
}


void WvFastString::newbuf(size_t size)
{
    buf = alloc(size);
    buf->links = 1;
    str = buf->data;
}


// If the string is linked to more than once, we need to make our own copy 
// of it.  If it was linked to only once, then it's already "unique".
WvString &WvString::unique()
{
    if (buf->links > 1 && str)
    {
	WvStringBuf *newb = alloc(len() + 1);
	memcpy(newb->data, str, newb->size);
	unlink();
	link(newb, newb->data);
    }
	    
    return *this; 
}


WvFastString &WvFastString::operator= (const WvFastString &s2)
{
    if (s2.buf == buf && s2.str == str)
	return *this; // no change
    else
    {
	unlink();
	link(s2.buf, s2.str);
    }
    return *this;
}


WvString &WvString::operator= (int i)
{
    unlink();
    newbuf(32);
    sprintf(str, "%d", i);
    return *this;
}


WvString &WvString::operator= (const WvFastString &s2)
{
    if (s2.buf == buf && s2.str == str)
	return *this; // no change
    else if (!s2.buf)
    {
	// assigning from a non-copied string - copy data if needed.
	unlink();
	link(&nullbuf, s2.str);
	unique();
    }
    else
    {
	// just a normal string link
	unlink();
	link(s2.buf, s2.str);
    }
    return *this;
}


// string comparison
bool WvFastString::operator== (WvStringParm s2) const
{
    return (str==s2.str) || (str && s2.str && !strcmp(str, s2.str));
}


bool WvFastString::operator!= (WvStringParm s2) const
{
    return (str!=s2.str) && (!str || !s2.str || strcmp(str, s2.str));
}


bool WvFastString::operator== (const char *s2) const
{
    return (str==s2) || (str && s2 && !strcmp(str, s2));
}


bool WvFastString::operator!= (const char *s2) const
{
    return (str!=s2) && (!str || !s2 || strcmp(str, s2));
}


// not operator is 'true' if string is empty
bool WvFastString::operator! () const
{
    return !str || !str[0];
}


// parse a 'percent' operator from a format string.  For example:
//        cptr      out:  zeropad  justify   maxlen  return pointer
//        "%s"             false      0         0    "s"
//        "%-15s"          false    -15         0    "s"
//        "%15.5s"         false     15         5    "s"
//        "%015.5s"        true      15         5    "s"
// and so on.  On entry, cptr should _always_ point at a percent '%' char.
//
static const char *pparse(const char *cptr,
			  bool &zeropad, int &justify, int &maxlen)
{
    assert(*cptr == '%');
    cptr++;

    zeropad = (*cptr == '0');

    justify = atoi(cptr);
    
    for (; *cptr && *cptr!='.' && *cptr!='%' && !isalpha(*cptr); cptr++)
	;
    if (!*cptr) return cptr;
    
    if (*cptr == '.')
	maxlen = atoi(cptr+1);
    else
	maxlen = 0;
    
    for (; *cptr && *cptr!='%' && !isalpha(*cptr); cptr++)
	;
    
    return cptr;
}


// Accept a printf-like format specifier (but more limited) and an array
// of WvStrings, and render them into another WvString.  For example:
//          WvString x[] = {"foo", "blue", 1234};
//          WvString ret = WvString::do_format("%s%10.2s%-10s", x);
//
// The 'ret' string will be:  "foo        bl1234      "
// Note that only '%s' is supported, though integers can be rendered
// automatically into WvStrings.  %d, %f, etc are not allowed!
//
// This function is usually called from some other function which allocates
// the array automatically.
//
void WvFastString::do_format(WvFastString &output, const char *format,
			     const WvFastString * const *a)
{
    static const char blank[] = "(nil)";
    const WvFastString * const *argptr = a;
    const char *iptr = format, *arg;
    char *optr;
    int total = 0, aplen, ladd, justify, maxlen;
    bool zeropad;
    
    // count the number of bytes we'll need
    while (*iptr)
    {
	if (*iptr != '%')
	{
	    total++;
	    iptr++;
	    continue;
	}
	
	// otherwise, iptr is at a percent expression
	iptr = pparse(iptr, zeropad, justify, maxlen);
	if (*iptr == '%') // literal percent
	{
	    total++;
	    iptr++;
	    continue;
	}
	
	assert(*iptr == 's' || *iptr == 'c');

	if (*iptr == 's')
	{
	    if (!*argptr || !(**argptr).cstr())
		arg = blank;
	    else
		arg = (**argptr).cstr();
	    ladd = _max(abs(justify), strlen(arg));
	    if (maxlen && maxlen < ladd)
		ladd = maxlen;
	    total += ladd;
	    argptr++;
	    iptr++;
	    continue;
	}
	
	if (*iptr++ == 'c')
	{
	    argptr++;
	    total++;
	}
    }
    
    output.setsize(total + 1);
    
    // actually render the final string
    iptr = format;
    optr = output.str;
    argptr = a;
    while (*iptr)
    {
	if (*iptr != '%')
	{
	    *optr++ = *iptr++;
	    continue;
	}
	
	// otherwise, iptr is at a "percent expression"
	iptr = pparse(iptr, zeropad, justify, maxlen);
	if (*iptr == '%')
	{
	    *optr++ = *iptr++;
	    continue;
	}
	if (*iptr == 's')
	{
	    if (!*argptr || !(**argptr).cstr())
		arg = blank;
	    else
		arg = (**argptr).cstr();
	    aplen = strlen(arg);
	    if (maxlen && maxlen < aplen)
		aplen = maxlen;
	
	    if (justify > aplen)
	    {
	        if (zeropad)
		    memset(optr, '0', justify-aplen);
		else
		    memset(optr, ' ', justify-aplen);
		optr += justify-aplen;
	    }
	
	    strncpy(optr, arg, aplen);
	    optr += aplen;
	
	    if (justify < 0 && -justify > aplen)
	    {
	        if (zeropad)
		    memset(optr, '0', -justify-aplen);
		else
		    memset(optr, ' ', -justify-aplen);
		optr += -justify - aplen;
	    }
	    
	    argptr++;
	    iptr++;
	    continue;
	}
	if (*iptr++ == 'c')
	{
	    arg = **argptr++;
	    *optr++ = (char)atoi(arg);
	        
	    argptr++;
	}
    }
    *optr = 0;
}


