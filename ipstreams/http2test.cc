/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvHttpPool test.  Downloads the urls given on stdin.
 */
#include "wvhttp.h"
#include "wvstreamlist.h"
#include "wvlog.h"
#include "wvfile.h"
#include "strutils.h"

// backwards compatibility
typedef WvURL WvUrl;

class WvHttpStream;

class WvBufStream : public WvStream
{
public:
    bool dead, eof;
    WvStream **death_notify;
    
    WvBufStream() { dead = eof = false; death_notify = NULL; }
    virtual ~WvBufStream();
	
    virtual void close() 
    {
	dead = true; 
	if (death_notify) *death_notify = NULL;
	death_notify = NULL;
    }
    
    virtual size_t uread(void *buf, size_t size)
        { if (inbuf.used()) return WvStream::uread(buf, size);
	  if (eof) close(); return 0; }
    virtual size_t uwrite(const void *buf, size_t size)
        { inbuf.put(buf, size); return size; }
    virtual bool isok() const { return !dead; }
    
    virtual bool pre_select(SelectInfo &si)  
    {
	return WvStream::pre_select(si) || si.wants.writable || eof;
    }
};


WvBufStream::~WvBufStream()
{
    close();
}


struct WvUrl2Stream
{
    WvUrl url;
    WvString headers;
    WvHttpStream *instream;
    WvBufStream *outstream;
    
    WvUrl2Stream(WvStringParm _url, WvStringParm _headers,
		 WvHttpStream *_instream)
	       : url(_url), headers(_headers) 
	{ 
	    instream = _instream;
	    WvBufStream *x = new WvBufStream;
	    outstream = x;
	    x->death_notify = (WvStream **)&outstream;
	}
    
    ~WvUrl2Stream() { if (outstream) outstream->death_notify = NULL; }
};

DeclareWvList(WvUrl2Stream);


class WvHttpStream : public WvStreamClone
{
    WvStream *cloned;
public:
    WvIPPortAddr remaddr;
    
private:
    WvLog log;
    WvTCPConn tcp;
    WvUrl2StreamList urls;
    
    WvUrl2Stream *curl; // current url
    size_t remaining;
    bool chunked, in_chunk_trailer;

public:
    WvHttpStream(const WvIPPortAddr &_remaddr);
    virtual ~WvHttpStream();
    virtual void close();
    
    void addurl(WvUrl2Stream *url);
    void doneurl();
    
    virtual void execute();
};

DeclareWvDict(WvHttpStream, WvIPPortAddr, remaddr);


class WvHttpPool : public WvStreamList
{
    WvLog log;
    WvResolver dns;
    WvHttpStreamDict conns;
    WvUrl2StreamList urls;
    
public:
    WvHttpPool();
    virtual ~WvHttpPool();
    
    virtual bool pre_select(SelectInfo &si);
    virtual void execute();
    
    WvStream *addurl(WvStringParm _url, WvStringParm _headers);
private:
    void unconnect(WvHttpStream *s);
    
public:
    bool idle() const 
        { return !urls.count(); }
};


static int numstreams = 0;

WvHttpStream::WvHttpStream(const WvIPPortAddr &_remaddr)
    : WvStreamClone(&cloned), remaddr(_remaddr),
	log(WvString("HTTP %s #%s", remaddr, ++numstreams), WvLog::Debug),
	tcp(remaddr)
{
    log("Opening server connection.\n");
    cloned = &tcp;
    curl = NULL;
    remaining = 0;
    chunked = in_chunk_trailer = false;
    
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvHttpStream::~WvHttpStream()
{
    log("Deleting.\n");
    if (isok())
	close();
    if (geterr())
	log("Error was: %s\n", errstr());
}


void WvHttpStream::close()
{
    log("Closing.\n");
    WvStreamClone::close();
    
    if (curl && curl->outstream)
	curl->outstream->seterr(EIO);
}


WvString fixnl(WvStringParm nonl)
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


void WvHttpStream::addurl(WvUrl2Stream *url)
{
    log("Adding a new url: '%s'\n", url->url);
    
    if (!url->url.isok())
	return;
    
    write(fixnl(WvString("GET %s HTTP/1.1\n"
			 "Host: %s:%s\n"
			 "Connection: keep-alive\n"
			 "%s"
			 "\n",
			 url->url.getfile(),
			 url->url.gethost(), url->url.getport(),
			 url->headers)));
    
    urls.append(url, false);
}


void WvHttpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);
    
    if (curl->outstream)
	curl->outstream->eof = true;
    curl = NULL;
    chunked = in_chunk_trailer = false;
    urls.unlink_first();
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
	    
	    WvUrl2Stream *url = urls.first();
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
	    
	    if (!line[0])
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
}


WvHttpPool::~WvHttpPool()
{
    if (geterr())
	log("Error was: %s\n", errstr());
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
    
    log("pre_select: main:%s conns:%s urls:%s\n",
         count(), conns.count(), urls.count());
    
    WvUrl2StreamList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (!i->outstream || !i->url.isok())
	{
	    //log("'%s' is dead: %s/%s\n", 
	    //	i->url, i->url.isok(), i.outstream->isok());
	    if (!i->url.isok())
	    {
		log("URL not okay: '%s'\n", i->url);
		if (i->outstream)
		    i->outstream->eof = true;
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
    
    WvUrl2StreamList::Iter i(urls);
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
	
	if (!s)
	{
	    s = new WvHttpStream(ip);
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


WvStream *WvHttpPool::addurl(WvStringParm _url, WvStringParm _headers)
{
    log("Adding a new url to pool: '%s'\n", _url);
    WvUrl2Stream *url = new WvUrl2Stream(_url, _headers, NULL);
    urls.append(url, true);
    
    return url->outstream;
}


void WvHttpPool::unconnect(WvHttpStream *s)
{
    log("Unconnecting stream to %s.\n", s->remaddr);
    
    WvUrl2StreamList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (i->instream == s)
	    i->instream = NULL;
    }
    
    unlink(s);
    conns.remove(s);
}



int main(int argc, char **argv)
{
    WvLog log("http2test", WvLog::Info);
    WvStreamList l;
    WvHttpPool p;
    WvString headers("");
    char *line;
    
    l.append(wvcon, false);
    l.append(&p, false);
    
    while (p.isok() && (wvcon->isok() || !p.idle()))
    {
	if (l.select(1000))
	{
	    l.callback();
	    
	    line = wvcon->getline(0);
	    if (line)
	    {
		line = trim_string(line);
		if (!line[0])
		    continue;
		else if (strstr(line, ": "))
		{
		    // an extra http header
		    headers = WvString("%s%s\n", headers, line);
		    continue;
		}
		
		WvStream *s = p.addurl(line, headers);
		if (s)
		{
		    static int num = 0;
		    WvFile *f = new WvFile(WvString("/tmp/url_%s", ++num), 
					   O_CREAT|O_WRONLY|O_TRUNC);
		    assert(!f->readable);
		    s->autoforward(*f);
		    l.append(s, true);
		    l.append(f, true);
		}
	    }
	}
    }
    
    if (!p.isok() && p.geterr())
	log("HttpPool: %s\n", p.errstr());
    
    return 0;
}
