/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvUrl is a simple URL-parsing class with built-in (though still somewhat
 * inconvenient) DNS resolution.
 * 
 * See wvurl.h.
 */ 
#include "wvurl.h"


// Split up the URL into a hostname, a port, and the rest of it.
WvUrl::WvUrl(WvStringParm url) : err("No error")
{
    WvString work(url);
    char *cptr, *wptr = work.edit();
    
    port = 0; // error condition by default
    addr = NULL;
    resolving = true;
    
    if (strncmp(wptr, "http://", 7)) // NOT equal
    {
	err = "WvUrl can only handle HTTP URLs.";
	return;
    }
    wptr += 7;
    
    hostname = wptr;
    
    cptr = strchr(wptr, '/');
    if (!cptr) // no path given
	file = "/";
    else
    {
	file = cptr;
	*cptr = 0;
    }
    
    cptr = strchr(wptr, ':');
    if (!cptr)
	port = 80;
    else
    {
	port = atoi(cptr+1);
	*cptr = 0;
    }

    hostname = wptr;
    
    resolve();
}


WvUrl::WvUrl(const WvUrl &url) : err("No error")
{
    addr = NULL;
    resolving = true;
    
    hostname = url.hostname;
    file = url.file;
    port = url.port;
    
    resolve();
}


WvUrl::~WvUrl()
{
    if (addr) delete addr;
}


bool WvUrl::resolve()
{
    const WvIPAddr *ip;
    int numaddrs;
    
    numaddrs = dns.findaddr(0, hostname, &ip);
    if (!numaddrs) // error condition
    {
	err = WvString("Host '%s' could not be found.", hostname);
	resolving = false;
	return false;
    }
    else if (numaddrs < 0) // still waiting
    {
	resolving = true;
	return false;
    }
    else // got at least one address
    {
	resolving = false;
	if (addr) delete addr;
	addr = new WvIPPortAddr(*ip, port);
	return true;
    }
}


// Print out the URL, using the port name (if it's not 80), and either the 
// hostname (if we know it) or the address (if we know that instead.)
WvUrl::operator WvString () const
{
    if (!isok())
	return WvString("(Invalid URL: %s)", err);
    
    WvString portstr("");
    if (port && port != 80)
	portstr = WvString(":%s", port);
    if (hostname)
	return WvString("http://%s%s%s", hostname, portstr, file);
    else if (addr)
	return WvString("http://%s%s%s", *addr, portstr, file);
    else
    {
	assert(0);
	return WvString("(Invalid URL)");
    }
}


