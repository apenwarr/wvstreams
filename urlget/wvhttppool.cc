/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A fast, easy-to-use, parallelizing, pipelining HTTP/1.1 file retriever.
 * 
 * See wvhttppool.h.
 */
#include "wvhttppool.h"
#include "wvbufstream.h"
#include "wvtcp.h"
#include "../crypto/wvsslstream.h"
#include "strutils.h"

bool WvHttpStream::enable_pipelining = true;
int WvHttpStream::max_requests = 100;


WvUrlRequest::WvUrlRequest(WvStringParm _url, WvStringParm _headers,
			   WvHttpStream *_instream)
    : url(_url), headers(_headers)
{ 
    instream = _instream;
    WvBufHttpStream *x = new WvBufHttpStream;
    outstream = x;
    x->death_notify = (WvStream **)&outstream;
}


WvUrlRequest::~WvUrlRequest()
{
    done();
}


void WvUrlRequest::done()
{
    if (outstream)
    {
	outstream->seteof();
	outstream->death_notify = NULL;
    }
    outstream = NULL; 
}


static WvString fixnl(WvStringParm nonl)
{
    WvBuffer b;
    const char *cptr;
    
    for (cptr = nonl; cptr && *cptr; cptr++)
    {
	if (*cptr == '\r')
	    continue;
	else if (*cptr == '\n')
	    b.put("\r", 1); // put BOTH \r and \n
	b.put(cptr, 1);
    }
    
    return b.getstr();
}


WvString WvUrlRequest::request_str(bool keepalive)
{
    return fixnl(WvString("GET %s HTTP/1.1\n"
			  "Host: %s:%s\n"
			  "Connection: %s\n"
			  "%s\n"
			  "\n",
			  url.getfile(),
			  url.gethost(), url.getport(),
			  keepalive ? "keep-alive" : "close",
			  trim_string(headers.edit())));
}


WvHttpStream::WvHttpStream(const WvIPPortAddr &_remaddr, bool ssl)
    : WvStreamClone(0), remaddr(_remaddr),
	log(WvString("HTTP %s", remaddr), WvLog::Debug)
{
    log("Opening server connection.\n");
    curl = NULL;
    remaining = 0;
    chunked = in_chunk_trailer = false;
    request_count = 0;
    
    cloned = new WvTCPConn(remaddr);
    if (ssl)
	cloned = new WvSSLStream(cloned);
    
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvHttpStream::~WvHttpStream()
{
    log("Deleting.\n");
    close();
    
    if (geterr())
	log("Error was: %s\n", errstr());
}


void WvHttpStream::close()
{
    log("Closing.\n");
    WvStreamClone::close();
    
    if (geterr())
    {
	// if there was an error, count the first URL as done.  This prevents
	// retrying indefinitely.
	if (!curl && !urls.isempty())
	    curl = urls.first();
	if (!curl && !waiting_urls.isempty())
	    curl = waiting_urls.first();
	if (curl)
	    log("URL '%s' is FAILED\n", curl->url);
	if (curl) 
	    curl->done();
    }
    
    if (curl)
	curl->done();
}


void WvHttpStream::addurl(WvUrlRequest *url)
{
    log("Adding a new url: '%s'\n", url->url);
    
    assert(url->outstream);
    
    if (!url->url.isok())
	return;
    
    waiting_urls.append(url, false);
    
    if (enable_pipelining || urls.isempty())
	request_next();
}


void WvHttpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);
    
    curl->done();
    curl = NULL;
    chunked = in_chunk_trailer = false;
    urls.unlink_first();
    
    if (urls.isempty())
	request_next();
}


void WvHttpStream::request_next()
{
    if (request_count < max_requests && !waiting_urls.isempty())
    {
	WvUrlRequest *url = waiting_urls.first();
	log("Making request #%s for URL %s\n", request_count+1, url->url);
	waiting_urls.unlink_first();
	request_count++;
	
	write(url->request_str(request_count < max_requests));
	urls.append(url, false);
    }
}


void WvHttpStream::execute()
{
    char buf[1024], *line;
    size_t len;
    
    WvStreamClone::execute();
    
    // make connections timeout after some idleness
    if (alarm_was_ticking)
    {
	log("urls count: %s\n", urls.count());
	if (!urls.isempty())
	{
	    seterr(ETIMEDOUT);
	    
	    WvUrlRequest *url = urls.first();
	    if (url->outstream)
		url->outstream->seterr(ETIMEDOUT);
	}
	else
	    close(); // timed out, but not really an error
	return;
    }
    
    if (!curl)
    {
	// in the header section
	line = getline(0);
	if (line)
	{
	    line = trim_string(line);
	    log("Header: '%s'\n", line);
	    
	    if (urls.isempty())
	    {
		seterr("unsolicited data from server!");
		return;
	    }
	    
	    if (!strncasecmp(line, "Content-length: ", 16))
		remaining = atoi(line+16);
	    if (!strncasecmp(line, "Transfer-Encoding: ", 19)
		    && strstr(line+19, "chunked"))
		chunked = true;

            if (line[0])
            {
                char *p;
                if ((p = strchr(line, ':')) != NULL)
                {
                    *p = 0;
		    p = trim_string(p+1);
		    struct WvHTTPHeader *h = new struct WvHTTPHeader(line, p);
		    urls.first()->outstream->headers.add(h, true);
                }
		else if (strncasecmp(line, "HTTP/", 5) == 0)
		{
		    char *p = strchr(line, ' ');
		    if (p)
		    {
			*p = 0;
			urls.first()->outstream->version = line+5;
			urls.first()->outstream->status = atoi(p+1);
		    }
		}
            }
            else
	    {
		// blank line is the beginning of data section
		curl = urls.first();
		in_chunk_trailer = false;
		log("Starting data: %s/%s\n", remaining, chunked);
	    }
	}
    }
    else if (chunked && !remaining)
    {
	line = getline(0);
	if (line)
	{
	    line = trim_string(line);
	    
	    if (in_chunk_trailer)
	    {
		// in the trailer section of a chunked encoding
		log("Trailer: '%s'\n", line);
		
		// a blank line means we're finally done!
		if (!line[0])
		    doneurl();
	    }
	    else
	    {
		// in the "length line" section of a chunked encoding
		if (line[0])
		{
		    remaining = (size_t)strtoul(line, NULL, 16);
		    if (!remaining)
			in_chunk_trailer = true;
		    log("Chunk length is %s ('%s').\n", remaining, line);
		}
	    }
	}
    }
    else // not chunked or currently in a chunk - read 'remaining' bytes.
    {
	// in the data section of a chunked or content-length encoding,
	// with 'remaining' bytes of data left.
	
	if (remaining > sizeof(buf))
	    len = read(buf, sizeof(buf));
	else
	    len = read(buf, remaining);
	remaining -= len;
	if (len)
	    log(WvLog::Debug5, 
		"Read %s bytes (%s bytes left).\n", len, remaining);
	if (curl->outstream)
	    curl->outstream->write(buf, len);
	
	if (!remaining && !chunked)
	    doneurl();
    }
    
    if (urls.isempty())
	alarm(5000); // just wait a few seconds before closing connection
    else
	alarm(60000); // give the server a minute to respond, if we're waiting
}



WvHttpPool::WvHttpPool() : log("HTTP Pool", WvLog::Debug), conns(10)
{
    log("Pool initializing.\n");
    num_streams_created = 0;
}


WvHttpPool::~WvHttpPool()
{
    log("Created %s individual HTTP sessions during this run.\n",
	num_streams_created);
    if (geterr())
	log("Error was: %s\n", errstr());
    
    // these must get zapped before the URL list, since they have pointers
    // to URLs.
    conns.zap();
}


bool WvHttpPool::pre_select(SelectInfo &si)
{
    bool sure = false;
    
    WvHttpStreamDict::Iter ci(conns);
    for (ci.rewind(); ci.next(); )
    {
	if (!ci->isok())
	{
	    unconnect(ci.ptr());
	    ci.rewind();
	    log("Selecting true because of a dead stream.\n");
	    sure = true;
	}
    }
    
    log(WvLog::Debug4, "pre_select: main:%s conns:%s urls:%s\n",
         count(), conns.count(), urls.count());
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (!i->outstream || !i->url.isok())
	{
	    //log("'%s' is dead: %s/%s\n", 
	    //	i->url, i->url.isok(), i.outstream->isok());
	    if (!i->url.isok())
	    {
		log("URL not okay: '%s'\n", i->url);
		i->done();
	    }
	    i.xunlink();
	    continue;
	}
	    
	if (!i->instream)
	{
	    log("Checking dns for '%s'\n", i->url.gethost());
	    if (i->url.resolve() || dns.pre_select(i->url.gethost(), si))
	    {
		log("Selecting true because of '%s'\n", i->url);
		sure = true;
	    }
	}
    }
    
    if (WvStreamList::pre_select(si))
    {
	//log("Selecting true because of list members.\n");
	sure = true;
    }
	
    return sure;
}


void WvHttpPool::execute()
{
    WvStreamList::execute();
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	WvHttpStream *s;
	
	if (!i->outstream || !i->url.isok() || !i->url.resolve())
	    continue; // skip it for now
	
	WvIPPortAddr ip(i->url.getaddr());
	s = conns[ip];
	//if (!s) log("conn for '%s' is not found.\n", ip);
	
	if (s && !s->isok())
	{
	    unconnect(s);
	    s = NULL;
	}
	
	if (!i->outstream)
	    continue; // unconnect might have caused this URL to be marked bad
	
	if (!s)
	{
	    num_streams_created++;
	    s = new WvHttpStream(ip, i->url.getproto() == "https");
	    conns.add(s, true);
	    
	    // add it to the streamlist, so it can do things
	    append(s, false);
	}
	
	if (!i->instream)
	{
	    s->addurl(i.ptr());
	    i->instream = s;
	}
    }
}


WvBufHttpStream *WvHttpPool::addurl(WvStringParm _url, WvStringParm _headers)
{
    log("Adding a new url to pool: '%s'\n", _url);
    WvUrlRequest *url = new WvUrlRequest(_url, _headers, NULL);
    urls.append(url, true);
    
    return url->outstream;
}


void WvHttpPool::unconnect(WvHttpStream *s)
{
    log("Unconnecting stream to %s.\n", s->remaddr);
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (i->instream == s)
	    i->instream = NULL;
    }
    
    unlink(s);
    conns.remove(s);
}
