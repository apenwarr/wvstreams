/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#include "wvhttp.h"
#include "strutils.h"
#include <assert.h>



WvHTTPStream::WvHTTPStream(const WvURL &_url)
	: WvStreamClone(NULL), headers(7), client_headers(7),
          url(_url)
{
    state = Resolving;
    num_received = 0;
    tcp = NULL;
    
    // we need this: if the URL tried to dns-resolve before, but failed,
    // this might make isok() true again if the name has turned up.
    url.resolve();
}


bool WvHTTPStream::isok() const
{
    if (cloned)
	return WvStreamClone::isok();
    else
	return url.isok();
}


int WvHTTPStream::geterr() const
{
    if (cloned)
	return WvStreamClone::geterr();
    else
	return -1;
}


WvString WvHTTPStream::errstr() const
{
    if (cloned)
	return WvStreamClone::errstr();
    else if (!url.isok())
	return url.errstr();
    else
	return "Unknown error! (no stream yet)";
}


bool WvHTTPStream::pre_select(SelectInfo &si)
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
	    cloned = tcp = new WvTCPConn(url.getaddr());
	}
	return false;

    case Connecting:
	tcp->select(0, false, true, false);
	if (!tcp->isconnected())
	    return false;
	if (tcp->geterr())
	    return false;

	// otherwise, we just finished connecting:  start transfer.
	state = ReadHeader1;
	delay_output(true);
	print("GET %s HTTP/1.0\r\n", url.getfile());
	print("Host: %s:%s\r\n", url.gethost(), url.getport());
        {
            WvHTTPHeaderDict::Iter i(client_headers);
            for (i.rewind(); i.next(); )
            {
                print("%s: %s\r\n", i().name, i().value);
            }
        }
        print("\r\n");
	delay_output(false);
	
	// FALL THROUGH!
	
    default:
	return WvStreamClone::isok()
	    && WvStreamClone::pre_select(si);
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
	line = trim_string(tcp->getline(0));
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
	line = trim_string(tcp->getline(0));
	if (line)
	{
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
	len = tcp->read(buf, count);
	num_received += len;
	return len;
	
    case Done:
	break;
    }
    
    return 0;
}
