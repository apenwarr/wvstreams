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
#include "wvsslstream.h"
#include "strutils.h"

bool WvHttpStream::global_enable_pipelining = true;
int WvUrlStream::max_requests = 100;

unsigned WvHash(const WvUrlStreamInfo &n)
{
    WvString key("%s%s", n.remaddr, n.proto);
    return (WvHash(key));
}


WvUrlRequest::WvUrlRequest(WvStringParm _url, WvStringParm _headers,
			   bool _pipeline_test, bool _headers_only)
    : url(_url), headers(_headers)
{ 
    instream = NULL;
    pipeline_test = _pipeline_test;
    headers_only = _headers_only;
    
    if (pipeline_test)
	outstream = NULL;
    else
    {
	WvBufUrlStream *x = new WvBufUrlStream;
	outstream = x;
	x->death_notify = (WvStream **)&outstream;
	x->url = url;
    }
    inuse = false;
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
    inuse = false;
    outstream = NULL; 
}


static WvString fixnl(WvStringParm nonl)
{
    WvDynamicBuffer b;
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
    return fixnl(WvString("%s %s HTTP/1.1\n"
			  "Host: %s:%s\n"
			  "Connection: %s\n"
			  "%s%s"
			  "\n",
			  headers_only ? "HEAD" : "GET",
			  url.getfile(),
			  url.gethost(), url.getport(),
			  keepalive ? "keep-alive" : "close",
			  trim_string(headers.edit()), !!headers ? "\n" : ""));
}


WvHttpStream::WvHttpStream(const WvIPPortAddr &_remaddr, bool _ssl,
			   WvIPPortAddrTable &_pipeline_incompatible)
    : WvUrlStream(_remaddr, WvString("HTTP %s", _remaddr)),
	pipeline_incompatible(_pipeline_incompatible)
{
    log("Opening server connection.\n");
    info.remaddr = _remaddr;
    curl = NULL;
    http_response = "";
    encoding = Unknown;
    remaining = 0;
    in_chunk_trailer = false;
    request_count = pipeline_test_count = 0;
    last_was_pipeline_test = false;
    
    enable_pipelining = global_enable_pipelining 
		&& !pipeline_incompatible[info.remaddr];
    ssl = _ssl;
    
    if (ssl)
    {
	info.proto = "https";
	cloned = new WvSSLStream(static_cast<WvFDStream*>(cloned));
    }
    else
	info.proto = "http";
    
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvHttpStream::~WvHttpStream()
{
    log(WvLog::Debug2, "Deleting.\n");
    close();
    
    if (geterr())
	log("Error was: %s\n", errstr());
}


void WvHttpStream::close()
{
    // assume pipelining is broken if we're closing without doing at least
    // one successful pipelining test and a following non-test request.
    if (enable_pipelining && max_requests > 1
	&& (pipeline_test_count < 1
	    || (pipeline_test_count==1 && last_was_pipeline_test)))
	pipelining_is_broken(2);
    
    if (isok())
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


void WvUrlStream::addurl(WvUrlRequest *url)
{
    log(WvLog::Debug4, "Adding a new url: '%s'\n", url->url);
    
    assert(url->outstream);
    
    if (!url->url.isok())
	return;
    
    waiting_urls.append(url, false);
    request_next();
}


void WvHttpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);
    
    last_was_pipeline_test = curl->pipeline_test;
    
    if (last_was_pipeline_test)
    {
	pipeline_test_count++;
	if (pipeline_test_count == 1)
	    start_pipeline_test(&curl->url);
	else if (pipeline_test_response != http_response)
	{
	    // getting a bit late in the game to be detecting brokenness :(
	    // However, if the response code isn't the same for both tests,
	    // something's definitely screwy.
	    pipelining_is_broken(4);
	    close();
	    return;
	}
	pipeline_test_response = http_response;
    }
    
    curl->done();
    curl = NULL;
    http_response = "";
    encoding = Unknown;
    in_chunk_trailer = false;
    remaining = 0;
    urls.unlink_first();
    request_next();
}


void WvHttpStream::send_request(WvUrlRequest *url, bool auto_free)
{
    request_count++;
    log("Request #%s: %s\n", request_count, url->url);
    write(url->request_str(url->pipeline_test
			   || request_count < max_requests));
    urls.append(url, auto_free);
}


void WvHttpStream::start_pipeline_test(WvUrl *url)
{
    WvUrl location(WvString(
		    "%s://%s:%s/wvhttp-pipeline-check-should-not-exist/",
		    url->getproto(), url->gethost(), url->getport()));
    WvUrlRequest *testurl = new WvUrlRequest(location, "", true, true);
    testurl->instream = this;
    send_request(testurl, true);
}


void WvHttpStream::request_next()
{
    // don't do a request if we've done too many already or we have none
    // waiting.
    if (request_count >= max_requests || waiting_urls.isempty())
	return;
    
    // don't do more than one request at a time if we're not pipelining.
    if (!enable_pipelining && !urls.isempty())
	return;
    
    // okay then, we really do want to send a new request.
    WvUrlRequest *url = waiting_urls.first();
    
    if (enable_pipelining && !request_count && max_requests > 1)
    {
	// start the pipelining compatibility test.
	start_pipeline_test(&url->url);
    }
    
    waiting_urls.unlink_first();
    send_request(url, false);
}


void WvHttpStream::pipelining_is_broken(int why)
{
    if (!pipeline_incompatible[info.remaddr])
    {
	pipeline_incompatible.add(new WvIPPortAddr(info.remaddr), true);
	log("Pipelining is broken on this server (%s)!  Disabling.\n", why);
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
	log(WvLog::Debug4, "urls count: %s\n", urls.count());
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

    // Die if somebody closed our outstream
    if (curl && !curl->outstream)
    {
	close();
	if (cloned)
	    delete cloned;
	cloned = new WvTCPConn(info.remaddr);
        if (ssl)
	    cloned = new WvSSLStream((WvTCPConn*)cloned);
	doneurl();
	return;
    }
    else if (curl)
	curl->inuse = true;

    if (!curl)
    {
	// in the header section
	line = getline(0);
	if (line)
	{
	    line = trim_string(line);
	    log(WvLog::Debug4, "Header: '%s'\n", line);
	    if (!http_response)
	    {
		http_response = line;
		
		// there are never two pipeline test requests in a row, so
		// a second response string exactly like the pipeline test
		// response implies that everything between the first and
		// second test requests was lost: bad!
		if (last_was_pipeline_test
		    && http_response == pipeline_test_response)
		{
		    pipelining_is_broken(1);
		    close();
		    return;
		}
		
		// http response #400 is "invalid request", which we
		// shouldn't be sending. If we get one of these right after
		// a test, it probably means the stuff that came after it
		// was mangled in some way during transmission ...and we
		// should throw it away.
		if (last_was_pipeline_test && !!http_response)
		{
		    const char *cptr = strchr(http_response, ' ');
		    if (cptr && atoi(cptr+1) == 400)
		    {
			pipelining_is_broken(3);
			close();
			return;
		    }
		}
	    }
	    
	    if (urls.isempty())
	    {
		log("got unsolicited data.\n");
		seterr("unsolicited data from server!");
		return;
	    }
	    
	    if (!strncasecmp(line, "Content-length: ", 16))
	    {
		remaining = atoi(line+16);
		encoding = ContentLength;
	    }
	    else if (!strncasecmp(line, "Transfer-Encoding: ", 19)
		    && strstr(line+19, "chunked"))
	    {
		encoding = Chunked;
	    }

            if (line[0])
            {
                char *p;
		WvBufUrlStream *outstream = urls.first()->outstream;
		
                if ((p = strchr(line, ':')) != NULL)
                {
                    *p = 0;
		    p = trim_string(p+1);
		    struct WvHTTPHeader *h = new struct WvHTTPHeader(line, p);
		    if (outstream)
			outstream->headers.add(h, true);
                }
		else if (strncasecmp(line, "HTTP/", 5) == 0)
		{
		    char *p = strchr(line, ' ');
		    if (p)
		    {
			*p = 0;
			if (outstream)
			{
			    outstream->version = line+5;
			    outstream->status = atoi(p+1);
			}
		    }
		}
            }
            else
	    {
		// blank line is the beginning of data section
		curl = urls.first();
		in_chunk_trailer = false;
		log(WvLog::Debug4,
		    "Starting data: %s (enc=%s)\n", remaining, encoding);
		
		if (encoding == Unknown)
		    encoding = Infinity; // go until connection closes itself

		if (curl->headers_only)
		{
		    log("Got all headers.\n");
//		    getline(0);
		    doneurl();
		}
	    }
	}
    }
    else if (encoding == Chunked && !remaining)
    {
	line = getline(0);
	if (line)
	{
	    line = trim_string(line);
	    
	    if (in_chunk_trailer)
	    {
		// in the trailer section of a chunked encoding
		log(WvLog::Debug4, "Trailer: '%s'\n", line);
		
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
		    log(WvLog::Debug4, "Chunk length is %s ('%s').\n",
			remaining, line);
		}
	    }
	}
    }
    else if (encoding == Infinity)
    {
	// just read data until the connection closes, and assume all was
	// well.  It sucks, but there's no way to tell if all the data arrived
	// okay... that's why Chunked or ContentLength encoding is better.
	len = read(buf, sizeof(buf));
	if (len)
	    log(WvLog::Debug5, "Infinity: read %s bytes.\n", len);
	if (curl->outstream)
	    curl->outstream->write(buf, len);
	
	if (!isok())
	    doneurl();
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
	
	if (!remaining && encoding == ContentLength)
	    doneurl();
    }
    
    if (urls.isempty())
	alarm(5000); // just wait a few seconds before closing connection
    else
	alarm(60000); // give the server a minute to respond, if we're waiting
}



WvHttpPool::WvHttpPool() : log("HTTP Pool", WvLog::Debug), conns(10),
				pipeline_incompatible(50)
{
    log("Pool initializing.\n");
    num_streams_created = 0;
}


WvHttpPool::~WvHttpPool()
{
    log("Created %s individual HTTP session(s) during this run.\n",
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
    
    WvUrlStreamDict::Iter ci(conns);
    for (ci.rewind(); ci.next(); )
    {
	if (!ci->isok() || urls.isempty())
	{
	    unconnect(ci.ptr());
	    ci.rewind();
	    log(WvLog::Debug3, "Selecting true because of a dead stream.\n");
	    sure = true;
	}
    }
    
    //log(WvLog::Debug4, "pre_select: main:%s conns:%s urls:%s\n",
    //     count(), conns.count(), urls.count());
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if ((!i->outstream && !i->inuse) || !i->url.isok())
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
	    log(WvLog::Debug4, "Checking dns for '%s'\n", i->url.gethost());
	    if (i->url.resolve() || dns.pre_select(i->url.gethost(), si))
	    {
		log(WvLog::Debug4, "Selecting true because of '%s'\n", i->url);
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
	WvUrlStream *s;
	
	if (!i->outstream || !i->url.isok() || !i->url.resolve())
	    continue; // skip it for now

	WvUrlStreamInfo ui;
	ui.remaddr = i->url.getaddr();
	ui.proto = "http";
	s = conns[ui];
	if (!s)
	{
	    ui.proto = "https";
	    s = conns[ui];
	}
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
	    s = new WvHttpStream(ui.remaddr, i->url.getproto() == "https",
				 pipeline_incompatible);
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


WvBufUrlStream *WvHttpPool::addurl(WvStringParm _url, WvStringParm _headers,
				    bool headers_only = false)
{
    log(WvLog::Debug4, "Adding a new url to pool: '%s'\n", _url);
    WvUrlRequest *url = new WvUrlRequest(_url, _headers, false, headers_only);
    urls.append(url, true);
    
    return url->outstream;
}


void WvHttpPool::unconnect(WvUrlStream *s)
{
    log("Unconnecting stream to %s.\n", s->info.remaddr);
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (i->instream == s)
	    i->instream = NULL;
    }
    
    unlink(s);
    conns.remove(s);
}
