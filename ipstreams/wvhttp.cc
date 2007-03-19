/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#include "wvhttp.h"
#include "wvsslstream.h"
#include "strutils.h"


WvHTTPStream::WvHTTPStream(const WvURL &_url)
    : WvStreamClone(NULL), url(_url), conn(NULL), state(Connecting),
      headers(7), client_headers(7)
{
    num_received = 0;

    WvTCPConn *tcp = new WvTCPConn(url.gethost(), url.getport());
    if (url.getproto() == "https")
    {
        WvSSLStream *ssl = new WvSSLStream(tcp, NULL, 0);
        setclone(ssl);        
        conn = ssl;
    }

    setclone(tcp);
    conn = tcp;
}


void WvHTTPStream::execute()
{
    WvStreamClone::execute();
    if (state == Connecting)
    {
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
    }
}


size_t WvHTTPStream::uread(void *buf, size_t count)
{
    size_t len;
    char *line;
    int retval;

    switch (state)
    {
    case Connecting:
        break;
    case ReadHeader1:
	line = trim_string(conn->getline());
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
	while ((line = trim_string(conn->getline())))
	{
	    if (!line[0])
            {
		state = ReadData;	// header is done
                return 0;
            }
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
	len = conn->read(buf, count);
	num_received += len;
	return len;
    case Done:
        break;
    }

    return 0;
}
