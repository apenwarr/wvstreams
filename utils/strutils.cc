/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Various useful string-based utilities.
 *
 */
#include "strutils.h"
#include "wvbuf.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
//#include <uuid.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#else
#undef errno
#define errno GetLastError()
#define strcasecmp _stricmp
#include <winsock2.h>
#endif

char *terminate_string(char *string, char c)
/**********************************************/
// Add character c to the end of a string after removing crlf's.
// NOTE: You need a buffer that's at least one character bigger than the
// current length of the string, including the terminating NULL.
{
    char *p;

    if (string == NULL)
    	return NULL;

    p = string + strlen(string) - 1;
    while (p >= string)
    {
        if (*p == '\r' || *p == '\n')
            --p;
        else
            break;
    }

    *(++p) = c;
    *(++p) = 0;

    return string;
}


char *trim_string(char *string)
/*********************************/
// Trims spaces off the front and end of strings.  Modifies the string.
// Specifically DOES allow string==NULL; returns NULL in that case.
{
    char *p;
    char *q;

    if (string == NULL)
    	return NULL;

    p = string;
    q = string + strlen(string) - 1;

    while (q >= p && isspace(*q))
    	*(q--) = 0;
    while (isspace(*p))
    	p++;

    return p;
}


char *trim_string(char *string, char c)
// Searches the string for c and removes it plus everything afterwards.
// Modifies the string and returns NULL if string == NULL.
{
    char *p;

    if (string == NULL)
        return NULL;

    p = string;

    while (*p != 0 && *p != c)
        p++;

    while (*p)
        *(p++) = 0;

    return string;
}


// return the string formed by concatenating string 'a' and string 'b' with
// the 'sep' character between them.  For example,
//    spacecat("xx", "yy", ";")
// returns "xx;yy".
// 
// This function is much faster than the more obvious WvString("%s;%s", a, b),
// so it's useful when you're producing a *lot* of string data.
WvString spacecat(WvStringParm a, WvStringParm b, char sep)
{
    int alen = a ? strlen(a) : 0;
    int blen = b ? strlen(b) : 0;
    WvString s;
    s.setsize(alen + blen + 2);
    char *cptr = s.edit();

    if (a)
	memcpy(cptr, a, alen);

    cptr[alen] = sep;

    if (b)
	memcpy(cptr + alen + 1, b, blen);

    cptr[alen + 1 + blen] = 0;
    
    return s;
}


// Replaces whitespace characters with nonbreaking spaces.
char *non_breaking(char * string)
{
    if (string == NULL)
        return (NULL);

    WvDynBuf buf;

    while (*string)
    {
        if (isspace(*string))
	    buf.putstr("&nbsp;");
        else 
	    buf.putch(*string);
        string++;
    }

    WvString s(buf.getstr());
    char *nbstr = new char[s.len() + 1];
    return strcpy(nbstr, s.edit());
}


// Searches _string (up to length bytes), replacing any occurrences of c1
// with c2.
void replace_char(void *_string, char c1, char c2, int length)
{
    char *string = (char *)_string;
    for (int i=0; i < length; i++)
    	if (*(string+i) == c1)
    	    *(string+i) = c2;
}

// Snip off the first part of 'haystack' if it consists of 'needle'.
char *snip_string(char *haystack, char *needle)
{
    if(!haystack)
        return NULL;
    if(!needle)
        return haystack;
    char *p = strstr(haystack, needle);
    if(!p || p != haystack)
        return haystack;
    else
        return haystack + strlen(needle);
}

#ifndef _WIN32
char *strlwr(char *string)
{
    char *p = string;
    while (*p)
    {
    	*p = tolower(*p);
    	p++;
    }

    return string;
}


char *strupr(char *string)
{
    char *p = string;
    while (*p)
    {
	*p = toupper(*p);
	p++;
    }

    return string;
}
#endif

// true if all the characters in "string" are isalnum().
bool is_word(const char *p)
{
    while (*p)
    {
    	if(!isalnum(*p++))
    	    return false;
    }
    
    return true;
}


// produce a hexadecimal dump of the data buffer in 'buf' of length 'len'.
// it is formatted with 16 bytes per line; each line has an address offset,
// hex representation, and printable representation.
WvString hexdump_buffer(const void *_buf, size_t len, bool charRep)
{
    const unsigned char *buf = (const unsigned char *)_buf;
    size_t count, count2, top;
    WvString out;

    out.setsize(len / 16 * 80 + 80);
    char *cptr = out.edit();
    
    for (count = 0; count < len; count+=16)
    {
	top = len-count < 16 ? len-count : 16;
	cptr += sprintf(cptr, "[%03X] ", (unsigned int)count);
	
	// dump hex values
	for (count2 = 0; count2 < top; count2++)
	{
	    if (count2 && !(count2 % 4))
		*cptr++ = ' ';
	    cptr += sprintf(cptr, "%02X", buf[count+count2]);
	}
	
	// print horizontal separation
	for (count2 = top; count2 < 16; count2++)
	{
	    if (count2 && !(count2 % 4))
	    {
		strcat(cptr, "   ");
		cptr += 3;
	    }
	    else
	    {
		strcat(cptr, "  ");
		cptr += 2;
	    }
	}
	
	*cptr++ = ' ';
	
	// dump character representation
	if (charRep)
	    for (count2 = 0; count2 < top; count2++)
	        *cptr++ = (isprint(buf[count+count2])
			   ? buf[count+count2] : '.');

	*cptr++ = '\n';
    }
    *cptr = 0;
    return out;
}


// return true if the character is a newline.
bool isnewline(char c)
{
    return c=='\n' || c=='\r';
}

// ex: WvString foo = web_unescape("I+am+text.%0D%0A");
WvString web_unescape(const char *str, bool no_space)
{
    const char *iptr;
    char *optr;
    char *idx1, *idx2;
    static const char hex[] = "0123456789ABCDEF";
    WvString in, intmp(str), out;
 
    in = trim_string(intmp.edit());
    out.setsize(strlen(in) + 1);

    optr = out.edit();
    for (iptr = in, optr = out.edit(); *iptr; iptr++)
    {
        if (*iptr == '+' && !no_space)
            *optr++ = ' ';
        else if (*iptr == '%' && iptr[1] && iptr[2])
        {
            idx1 = strchr(hex, toupper((unsigned char) iptr[1]));
            idx2 = strchr(hex, toupper((unsigned char) iptr[2]));

            if (idx1 && idx2)
                *optr++ = ((idx1 - hex) << 4) | (idx2 - hex);

            iptr += 2;
        }
        else
            *optr++ = *iptr;
    }

    *optr = 0;

    return out;
}


// And it's magic companion: url_encode
WvString url_encode(WvStringParm stuff)
{
    unsigned int i;
    WvDynBuf retval;

    for (i=0; i < stuff.len(); i++)
    {
        if (isalnum(stuff[i]) || strchr("/_.-~", stuff[i]))
        {
            retval.put(&stuff[i], 1);
        }               
        else            
        {               
            char buf[3];
            sprintf(buf, "%%%02x", stuff[i] & 0xff);
            retval.put(&buf, 3);
        }
    }
    return retval.getstr();
}

WvString diff_dates(time_t t1, time_t t2)
{
    char out[25]; //Should be more then enough
    double diff = difftime(t1, t2);
    if(diff < 0)
        diff = -diff;
    if(diff > (60 * 60 * 24))
        //give a touch more granularity then the rest
        sprintf(out, "%.1f day(s)", diff / (60 * 60 * 24));
    else if(diff > (60 * 60)) 
        sprintf(out, "%.0f hour(s)", diff / (60 * 60));
    else if(diff > 60)
        sprintf(out, "%.0f minute(s)", diff / 60);
    else
        sprintf(out, "%.0f second(s)", diff);
    return out;
}

WvString rfc822_date(time_t when)
{
    WvString out;
    out.setsize(80);

    if (when < 0)
        when = time(NULL);

    struct tm *tmwhen = localtime(&when);
    strftime(out.edit(), 80, "%a, %d %b %Y %H:%M:%S %z", tmwhen);

    return out;
}


WvString backslash_escape(WvStringParm s1)
{
    // stick a backslash in front of every !isalnum() character in s1
    if (!s1)
        return "";

    WvString s2;
    s2.setsize(s1.len() * 2 + 1);

    const char *p1 = s1;
    char *p2 = s2.edit();
    while (*p1)
    {
        if (!isalnum(*p1))
            *p2++ = '\\';
        *p2++ = *p1++;
    }
    *p2 = 0;

    return s2;
}


int strcount(WvStringParm s, const char c)
{
    int n=0;
    const char *p = s;
    while ((p=strchr(p, c)) != NULL && p++)
        n++;

    return n;
}


WvString encode_hostname_as_DN(WvStringParm hostname)
{
    WvString dn("");
    
    WvStringList fqdnlist;
    WvStringList::Iter i(fqdnlist);
    
    fqdnlist.split(hostname, ".");
    for (i.rewind(); i.next(); )
	dn.append("dc=%s,", *i);
    dn.append("cn=%s", hostname);
    
    return dn;
}


WvString nice_hostname(WvStringParm name)
{
    WvString nice;
    char *optr, *optr_start;
    const char *iptr;
    bool last_was_dash;
    
    nice.setsize(name.len() + 2);

    iptr = name;
    optr = optr_start = nice.edit();
    if (!isascii(*iptr) || !isalnum(*(const unsigned char *)iptr))
	*optr++ = 'x'; // DNS names must start with a letter!
    
    last_was_dash = false;
    for (; *iptr; iptr++)
    {
	if (!isascii(*iptr))
	    continue; // skip it entirely
	
	if (*iptr == '-' || *iptr == '_')
	{
	    if (last_was_dash)
		continue;
	    last_was_dash = true;
	    *optr++ = '-';
	}
	else if (isalnum(*(const unsigned char *)iptr) || *iptr == '.')
	{
	    *optr++ = *iptr;
	    last_was_dash = false;
	}
    }
    
    if (optr > optr_start && !isalnum(*(const unsigned char *)(optr-1)))
	*optr++ = 'x'; // must _end_ in a letter/number too!
    
    *optr++ = 0;
    
    if (!nice.len())
	return "UNKNOWN";
    
    return nice;
}


WvString getfilename(WvStringParm fullname)
{
    WvString tmp(fullname);
    char *cptr = strrchr(tmp.edit(), '/');
    
    if (!cptr) // no slash at all
	return fullname;
    else if (!cptr[1]) // terminating slash
    {
	*cptr = 0;
	return getfilename(tmp);
    }
    else // no terminating slash
	return cptr+1;
}


WvString getdirname(WvStringParm fullname)
{
    WvString tmp(fullname);
    char *cptr = strrchr(tmp.edit(), '/');
    
    if (!cptr) // no slash at all
	return ".";
    else if (!cptr[1]) // terminating slash
    {
	*cptr = 0;
	return getdirname(tmp);
    }
    else // no terminating slash
    {
	*cptr = 0;
	return !tmp ? WvString("/") : tmp;
    }
}

// This function helps sizetoa() and sizektoa() below.  It takes a
// bunch of digits, and the default unit (indexed by size); and turns
// them into a WvString that's formatted to human-readable rounded
// sizes, with one decimal place.
WvString _sizetoa(unsigned long long digits, int size = 0)
{
    // Programmatically determine the units.  In order, these are:
    // bytes, kilobytes, megabytes, gigabytes, terabytes, petabytes,
    // exabytes, zettabytes, yottabytes.  Note that these are SI
    // prefixes, not binary ones.
    static char* size_name[] = { "B", "KB", "MB", "GB", "TB",
				 "PB", "EB", "ZB", "YB", NULL };

    // Let's loop, until we can get our "units" to less-than 1000.
    // But we also have to keep enough significant figures for two
    // decimal places, in order to round properly.  Therefore, we
    // should stop looping at 1000000.
    while (digits >= 1000000) {
	// We'll make an exception for 1000 TB and higher, since we
	// have no larger unit.  Well, I guess we can use exabytes,
	// but that's a little futher in the future.
	if (size_name[size + 1] == NULL)
	    break;

	// OK.  Let's go up another unit.
	digits /= 1000;
	size += 1;
    }

    // Now we can perform our rounding calculation.  We use the
    // algorithm derived from grade school.  If it is a 5 or higher,
    // round up.
    unsigned long long units = digits / 1000;
    unsigned tenths = digits % 1000 / 100;
    unsigned hundredths = digits % 100 / 10;
    if (hundredths >= 5)
	tenths += 1;
    if (tenths >= 10) {
	tenths = 0;
	units += 1;
    }
    if (units >= 1000 && (size_name[size + 1] != NULL)) {
	units /= 1000;
	tenths = 0;
	size += 1;
    }

    // Now we can return our result.
    return WvString("%s.%s %s", units, tenths, size_name[size]);
}

WvString sizetoa(long long blocks, int blocksize)
{
    unsigned long long bytes = blocks * blocksize;

    // Test if we are dealing in just bytes.  Plus, we should ensure
    // that we didn't overflow.  (Although that is highly unlikely,
    // with a 64-bit integer.)
    if ((bytes < 1000) && !(blocks * blocksize / 1000))
        return WvString("%s bytes", blocks * blocksize);

    return _sizetoa(bytes, 1);
}


WvString sizektoa(unsigned int kbytes)
{
    // Test if we are dealing in just kilobytes.
    if (kbytes < 1000)
        return WvString("%s KB", kbytes);

    return _sizetoa(kbytes, 2);
}

WvString strreplace(WvStringParm s, WvStringParm a, WvStringParm b)
{
    WvDynBuf buf;
    const char *sptr = s, *eptr;
    
    while ((eptr = strstr(sptr, a)) != NULL)
    {
	buf.put(sptr, eptr-sptr);
	buf.putstr(b);
	sptr = eptr + strlen(a);
    }
    
    buf.put(sptr, strlen(sptr));
    
    return buf.getstr();
}

WvString undupe(WvStringParm s, char c)
{
    WvDynBuf out;

    bool last = false;

    for (int i = 0; s[i] != '\0'; i++)
    {
        if (s[i] != c)
        {
            out.putch(s[i]);
            last = false;
        }
        else if (!last)
        {
            out.putch(c);
            last = true;
        }
    }
    
    return out.getstr();
}


WvString rfc1123_date(time_t t)
{
    struct tm *tm = gmtime(&t);
    WvString s;

    s.setsize(128);
    strftime(s.edit(), 128, "%a, %d %b %Y %H:%M:%S GMT", tm);

    return s;
}


int lookup(const char *str, const char * const *table, bool case_sensitive)
{
    for (int i = 0; table[i]; ++i)
    {
        if (case_sensitive)
        {
            if (strcmp(str, table[i]) != 0)
                continue;
        }
        else
        {
            if (strcasecmp(str, table[i]) != 0)
                continue;
        }
        return i;
    }
    return -1;
}


WvString hostname()
{
    int maxlen = 0;
    for (;;)
    {
        maxlen += 80;
        char *name = new char[maxlen];
        int result = gethostname(name, maxlen);
        if (result == 0)
        {
            WvString hostname(name);
            deletev name;
            return hostname;
        }
#ifdef _WIN32
	assert(errno == WSAEFAULT);
#else
        assert(errno == EINVAL);
#endif
    }
}


WvString fqdomainname() 
{
    struct hostent *myhost;

    myhost = gethostbyname(hostname());
    if (myhost)
	return myhost->h_name;
    else
	return WvString::null;
}


WvString metriculate(const off_t i)
{
    WvString res;
    int digits=0;
    int digit=0;
    long long int j=i;
    char *p;

    while (j)
    {
        digits++;
        j/=10;
    }

    j=i;
    // setsize says it takes care of the terminating NULL char
    res.setsize(digits + ((digits - 1) / 3) + ((j < 0) ? 1 : 0));
    p = res.edit();
    if (j < 0)
    {
        *p++ = '-';
        j = -j;
    }

    p += digits + ((digits - 1) / 3);
    *p-- = '\0';

    for (digit=0; digit<digits; digit++)
    {
        *p-- = '0' + ( j%10 );
        if (((digit+1) % 3) == 0 && digit < digits - 1)
            *p-- = ' ';
        j /= 10;
    }

    return res;
}


WvString afterstr(WvStringParm line, WvStringParm a)
{
    char *loc = strstr(line, a);
    if (loc == 0)
	return "";

    loc += a.len();
    WvString ret = loc;
    ret.unique();
    return ret;
}


WvString beforestr(WvStringParm line, WvStringParm a)
{
    WvString ret = line;
    ret.unique();    
    char *loc = strstr(ret, a);

    if (loc == 0)
	return line;

    loc[0] = '\0';
    return ret;
}


WvString substr(WvString line, unsigned int pos, unsigned int len)
{
    const char *tmp = line.cstr();
    if (pos > line.len()-1)
	return "";
    tmp += pos;

    WvString ret = tmp;
    char *tmp2 = ret.edit();
    if (pos + len < line.len())
	tmp2[len] = '\0';

    return ret;
}
