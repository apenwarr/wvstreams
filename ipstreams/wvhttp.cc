/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#include "wvhttp.h"
#include "strutils.h"
#include <assert.h>


//////////////////////////////////////// WvURL


// Split up the URL into a hostname, a port, and the rest of it.
WvURL::WvURL(const WvString &url) : err("No error")
{
    WvString work(url);
    char *cptr, *wptr = work.edit();
    
    port = 0; // error condition by default
    addr = NULL;
    resolving = true;
    
    if (strncmp(wptr, "http://", 7)) // NOT equal
    {
	err = "WvURL can only handle HTTP URLs.";
	return;
    }
    wptr += 7;
    
    hostname = wptr;
    hostname.unique();
    
    cptr = strchr(wptr, '/');
    if (!cptr) // no path given
	file = "/";
    else
    {
	file = cptr;
	file.unique();
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
    hostname.unique();
    
    resolve();
}


WvURL::~WvURL()
{
    if (addr) delete addr;
}


bool WvURL::resolve()
{
    const WvIPAddr *ip;
    int numaddrs;
    
    numaddrs = dns.findaddr(0, hostname, &ip);
    if (!numaddrs) // error condition
    {
	err = WvString("Host %s could not be found.", hostname);
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
WvURL::operator WvString () const
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



//////////////////////////////////////// WvHTTPStream



WvHTTPStream::WvHTTPStream(WvURL &_url)
	: WvStreamClone((WvStream **)&http), headers(7), url(_url)
{
    state = Resolving;
    http = NULL;
    num_received = 0;
    
    // we need this: if the URL tried to dns-resolve before, but failed,
    // this might make isok() true again if the name has turned up.
    url.resolve();
}


WvHTTPStream::~WvHTTPStream()
{
    if (http) delete http;
}


bool WvHTTPStream::isok() const
{
    if (http)
	return WvStreamClone::isok();
    else
	return url.isok();
}


int WvHTTPStream::geterr() const
{
    if (http)
	return WvStreamClone::geterr();
    else
	return -1;
}


const char *WvHTTPStream::errstr() const
{
    if (http)
	return WvStreamClone::errstr();
    else if (!url.isok())
	return url.errstr();
    else
	return "Unknown error!";
}


bool WvHTTPStream::select_setup(SelectInfo &si)
{
    if (!isok()) return false;
    
    switch (state)
    {
    case Resolving:
	if (!url.isok())
	    seterr("Invalid URL");
	else if (url.resolve())
	{
	    state = Connecting;
	    http = new WvTCPConn(url.getaddr());
	}
	return false;

    case Connecting:
	http->select(0, false, true, false);
	if (!http->isconnected())
	    return false;

	// otherwise, we just finished connecting:  start transfer.
	state = ReadHeader1;
	print("GET %s HTTP/1.0\n\n", url.getfile());
	
	// FALL THROUGH!
	
    default:
	return WvStreamClone::isok()
	    && WvStreamClone::select_setup(si);
    }
}


size_t WvHTTPStream::uread(void *buf, size_t count)
{
    char *line;
    int retval;
    size_t len;
    
    switch (state)
    {
    case Resolving:
    case Connecting:
	break;
	
    case ReadHeader1:
	line = http->getline(0);
	line = trim_string(line);
	if (line) // got response code line
	{
	    if (strncmp(line, "HTTP/", 5))
	    {
		seterr("Invalid HTTP response");
		return 0;
	    }
	    
	    retval = atoi(trim_string(line+9));
	    
	    if (retval / 100 != 2)
	    {
		seterr(WvString("HTTP error: %s", trim_string(line+9)));
		return 0;
	    }
		
	    state = ReadHeader;
	}
	break;
	
    case ReadHeader:
	line = http->getline(0);
	if (line)
	{
	    line = trim_string(line);
	    if (!line[0])
		state = ReadData;	// header is done
	    else
	    {
		char *cptr = strchr(line, ':');
		if (!cptr)
		    headers.add(new WvHTTPHeader(line, ""), true);
		else
		{
		    *cptr++ = 0;
		    line = trim_string(line);
		    cptr = trim_string(cptr);
		    headers.add(new WvHTTPHeader(line, cptr), true);
		}
	    }
	}
	break;
	
    case ReadData:
	len = http->read(buf, count);
	num_received += len;
	return len;
    }
    
    return 0;
}
