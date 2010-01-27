#include "wvstrutils.h"
#include "wvneeds-sockets.h"
#include <errno.h>
#ifndef _WIN32
#include <netdb.h>
#endif

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


