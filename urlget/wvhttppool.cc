/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A fast, easy-to-use, parallelizing, pipelining HTTP/1.1 file retriever.
 * 
 * See wvhttppool.h.
 */
#include <ctype.h>
#include <time.h>
#include "wvhttppool.h"
#include "wvbufstream.h"
#include "wvtcp.h"
#include "wvsslstream.h"
#include "strutils.h"
#include "fileutils.h"

bool WvHttpStream::global_enable_pipelining = true;
int WvUrlStream::max_requests = 100;

unsigned WvHash(const WvUrlStream::Target &n)
{
    WvString key("%s%s", n.remaddr, n.username);
    return (WvHash(key));
}


WvUrlRequest::WvUrlRequest(WvStringParm _url, WvStringParm _headers,
			   bool _pipeline_test, bool _headers_only)
    : url(_url), headers(_headers)
{ 
    instream = NULL;
    putstream = NULL;
    create_dirs = false;
    pipeline_test = _pipeline_test;
    headers_only = _headers_only;
    is_dir = false;    // for ftp primarily; set later
    
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


WvUrlRequest::WvUrlRequest(WvStringParm _url, WvStringParm _headers,
			   WvStream *s, bool _create_dirs)
    : url(_url), headers(_headers), putstream(s)
{
    create_dirs = _create_dirs;
    instream = NULL;
    pipeline_test = false;
    headers_only = false;
    is_dir = false;

    WvBufUrlStream *x = new WvBufUrlStream;
    outstream = x;
    x->death_notify = (WvStream **)&outstream;
    x->url = url;
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
	outstream->death_notify = NULL;
	outstream->seteof();
        outstream = NULL; 
    }
    inuse = false;
}


static WvString fixnl(WvStringParm nonl)
{
    WvDynBuf b;
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
    WvString request;
    if (!strncasecmp(url.getproto(), "http", 4))
	request = fixnl(WvString("%s %s HTTP/1.1\n"
				 "Host: %s:%s\n"
				 "Connection: %s\n"
				 "%s%s"
				 "\n",
				 headers_only ? "HEAD" : "GET",
				 url.getfile(),
				 url.gethost(), url.getport(),
				 keepalive ? "keep-alive" : "close",
				 trim_string(headers.edit()),
				 !!headers ? "\n" : ""));
    return request;
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


void WvUrlStream::delurl(WvUrlRequest *url)
{
    log(WvLog::Debug4, "Removing a url: '%s'\n", url->url);

    if (url == curl)
        doneurl();
    waiting_urls.unlink(url);
    urls.unlink(url);
}


WvHttpStream::WvHttpStream(const WvIPPortAddr &_remaddr, bool _ssl,
			   WvIPPortAddrTable &_pipeline_incompatible)
    : WvUrlStream(_remaddr, WvString("HTTP %s", _remaddr), ""),
      pipeline_incompatible(_pipeline_incompatible)
{
    log("Opening server connection.\n");
    http_response = "";
    encoding = Unknown;
    remaining = 0;
    in_chunk_trailer = false;
    pipeline_test_count = 0;
    last_was_pipeline_test = false;
    
    enable_pipelining = global_enable_pipelining 
		&& !pipeline_incompatible[target.remaddr];
    ssl = _ssl;
    
    if (ssl)
	cloned = new WvSSLStream(static_cast<WvFDStream*>(cloned));
    
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvHttpStream::~WvHttpStream()
{
    log(WvLog::Debug2, "Deleting.\n");
    if (geterr())
	log("Error was: %s\n", errstr());
    close();
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
        WvUrlRequest *msgurl = curl;
	if (!msgurl && !urls.isempty())
	    msgurl = urls.first();
	if (!msgurl && !waiting_urls.isempty())
	    msgurl = waiting_urls.first();
	if (msgurl)
	    log("URL '%s' is FAILED\n", msgurl->url);
    }
    waiting_urls.zap();
    if (curl)
        doneurl();
}


void WvHttpStream::doneurl()
{
    assert(curl != NULL);
    log("Done URL: %s\n", curl->url);
    
    http_response = "";
    encoding = Unknown;
    in_chunk_trailer = false;
    remaining = 0;
    
    last_was_pipeline_test = curl->pipeline_test;
    bool broken = false;
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
            broken = true;
	}
	pipeline_test_response = http_response;
    }
    
    assert(curl == urls.first());
    curl->done();
    curl = NULL;
    urls.unlink_first();
    
    if (broken)
        close();

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
    if (!pipeline_incompatible[target.remaddr])
    {
	pipeline_incompatible.add(new WvIPPortAddr(target.remaddr), true);
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
        if (curl)
            doneurl();
	if (cloned)
	    delete cloned;
	cloned = new WvTCPConn(target.remaddr);
        if (ssl)
	    cloned = new WvSSLStream((WvTCPConn*)cloned);
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


WvFtpStream::WvFtpStream(const WvIPPortAddr &_remaddr, WvStringParm _username,
			 WvStringParm _password)
    : WvUrlStream(_remaddr, WvString("HTTP %s", _remaddr), _username)
{
    data = NULL;
    logged_in = false;
    password = _password;
    uses_continue_select = true;
    last_request_time = time(0);
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvFtpStream::~WvFtpStream()
{
    close();
}


void WvFtpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);
    
    curl->done();
    curl = NULL;
    if (data)
    {
	delete data;
	data = NULL;
    }
    urls.unlink_first();
    last_request_time = time(0);
    alarm(60000);
    request_next();
}


void WvFtpStream::request_next()
{
    // don't do a request if we've done too many already or we have none
    // waiting.
    if (request_count >= max_requests || waiting_urls.isempty())
	return;
    
    if (!urls.isempty())
	return;
    
    // okay then, we really do want to send a new request.
    WvUrlRequest *url = waiting_urls.first();
    
    waiting_urls.unlink_first();

    request_count++;
    log("Request #%s: %s\n", request_count, url->url);
    urls.append(url, false);
    alarm(0);
}


void WvFtpStream::close()
{
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


char *WvFtpStream::get_important_line(int timeout)
{
    char *line;
    do
    {
	line = getline(timeout);
	if (!line)
	    return NULL;
    }
    while (line[3] == '-');
    return line;
}


bool WvFtpStream::pre_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;

    if (WvUrlStream::pre_select(si))
        return true;

    if (data && data->pre_select(si))
        return true;

    if (curl && curl->putstream && curl->putstream->pre_select(si))
        return true;

    si.wants = oldwant;

    return false;
}


bool WvFtpStream::post_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;

    if (WvUrlStream::post_select(si))
        return true;

    if (data && data->post_select(si))
        return true;

    if (curl && curl->putstream && curl->putstream->post_select(si))
        return true;

    si.wants = oldwant;

    return false;
}


void WvFtpStream::execute()
{
    char buf[1024], *line;
    WvStreamClone::execute();

    if (alarm_was_ticking && ((last_request_time + 60) <= time(0)))
    {
	log(WvLog::Debug4, "urls count: %s\n", urls.count());
	if (urls.isempty())
	    close(); // timed out, but not really an error

	return;
    }

    if (!logged_in)
    {
	line = get_important_line(60000);
	if (!line)
	    return;
	if (strncmp(line, "220", 3))
	{
	    log("Server rejected connection: %s\n", line);
	    seterr("server rejected connection");
	    return;
	}

	log(WvLog::Debug5, "Got greeting: %s\n", line);
	write(WvString("USER %s\r\n",
		       !target.username ? "anonymous" :
		       target.username.cstr()));
	line = get_important_line(60000);
	if (!line)
	    return;

	if (!strncmp(line, "230", 3))
	{
	    log(WvLog::Debug5, "Server doesn't need password.\n");
	    logged_in = true;        // No password needed;
	}
	else if (!strncmp(line, "33", 2))
	{
	    write(WvString("PASS %s\r\n", !password ? DEFAULT_ANON_PW :
		      password));
	    line = get_important_line(60000);
	    if (!line)
		return;
	    
	    if (line[0] == '2')
	    {
		log(WvLog::Debug5, "Authenticated.\n");
		logged_in = true;
	    }
	    else
	    {
		log("Strange response to PASS command: %s\n", line);
		seterr("strange response to PASS command");
		return;
	    }
	}
	else
	{
	    log("Strange response to USER command: %s\n", line);
	    seterr("strange response to USER command");
	    return;
	}

	write("TYPE I\r\n");
	line = get_important_line(60000);
	if (!line)
	    return;

	if (strncmp(line, "200", 3))
	{
	    log("Strange response to TYPE I command: %s\n", line);
	    seterr("strange response to TYPE I command");
	    return;
	}
    }

    if (!curl && !urls.isempty())
    {
	curl = urls.first();

	write(WvString("CWD %s\r\n", curl->url.getfile()));
	line = get_important_line(60000);
	if (!line)
	    return;

	if (!strncmp(line, "250", 3))
	{
	    log(WvLog::Debug5, "This is a directory.\n");
	    curl->is_dir = true;
	}

	write("PASV\r\n");
	line = get_important_line(60000);
	if (!line)
	    return;

	WvIPPortAddr *dataip = parse_pasv_response(line);

	if (!dataip)
	    return;

	log(WvLog::Debug4, "Data port is %s.\n", *dataip);
	// Open data connection.
	data = new WvTCPConn(*dataip);
	if (!data)
	{
	    log("Can't open data connection.\n");
	    seterr("can't open data connection");
	    return;
	}

	if (curl->is_dir)
	{
	    if (!curl->putstream)
	    {
		write(WvString("LIST %s\r\n", curl->url.getfile()));
		if (curl->outstream)
		{
		    WvString url_no_pw("ftp://%s%s%s%s", curl->url.getuser(),
				       !!curl->url.getuser() ? "@" : "", 
				       curl->url.gethost(),
				       curl->url.getfile());
		    curl->outstream->write(
			WvString(
			    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML "
			    "4.01//EN\">\n"
			    "<html>\n<head>\n<title>%s</title>\n"
			    "<meta http-equiv=\"Content-Type\" "
			    "content=\"text/html; "
			    "charset=ISO-8859-1\">\n"
			    "<base href=\"%s\"/>\n</head>\n"
			    "<style type=\"text/css\">\n"
			    "img { border: 0; padding: 0 2px; vertical-align: "
			    "text-bottom; }\n"
			    "td  { font-family: monospace; padding: 2px 3px; "
			    "text-align: right; vertical-align: bottom; }\n"
			    "td:first-child { text-align: left; padding: "
			    "2px 10px 2px 3px; }\n"
			    "table { border: 0; }\n"
			    "a.symlink { font-style: italic; }\n"
			    "</style>\n<body>\n"
			    "<h1>Index of %s</h1>\n"
			    "<hr/><table>\n", url_no_pw, curl->url, url_no_pw 
			    ));
		}
	    }
	    else
	    {
		log("Target is a directory.\n");
		seterr("target is a directory");
		doneurl();
	    }
	}
	else if (!curl->putstream)
	    write(WvString("RETR %s\r\n", curl->url.getfile()));
	else
	{
	    if (curl->create_dirs)
	    {
		write(WvString("CWD %s\r\n", getdirname(curl->url.getfile())));
		line = get_important_line(60000);
		if (!line)
		    return;

		if (strncmp(line, "250", 3))
		{
		    log("Path doesn't exist; creating directories...\n");
		    // create missing directories.
		    WvString current_dir("");
		    WvStringList dirs;
		    dirs.split(getdirname(curl->url.getfile()), "/");
		    WvStringList::Iter i(dirs);
		    for (i.rewind(); i.next(); )
		    {
			current_dir.append(WvString("/%s", i()));
			write(WvString("MKD %s\r\n", current_dir));
			line = get_important_line(60000);
			if (!line)
			    return;
		    }
		}
	    }
	    write(WvString("STOR %s\r\n", curl->url.getfile()));
	}

	log(WvLog::Debug5, "Waiting for response to STOR/RETR/LIST\n");
	line = get_important_line(60000);
	if (!line)
	    doneurl();
	else if (strncmp(line, "150", 3))
	{
	    log("Strange response to %s command: %s\n", 
		curl->putstream ? "STOR" : "RETR", line);
	    seterr(WvString("strange response to %s command",
			    curl->putstream ? "STOR" : "RETR"));
	    doneurl();
	}

    }

    if (curl)
    {
	if (curl->is_dir)
	{
	    line = data->getline(0);
	    if (line && curl->outstream)
	    {
		WvString output_line(parse_for_links(line));
		if (!!output_line)
		    curl->outstream->write(output_line);
		else
		    curl->outstream->write("Unknown format of LIST "
					   "response\n");
	    }
	}
	else
	{
	    if (curl->putstream)
	    {
		int len = curl->putstream->read(buf, sizeof(buf));
		log(WvLog::Debug5, "Read %s bytes.\n", len);

		if (len)
		    data->write(buf, len);
	    }
	    else
	    {
		int len = data->read(buf, sizeof(buf));
		log(WvLog::Debug5, "Read %s bytes.\n", len);

		if (len && curl->outstream)
		    curl->outstream->write(buf, len);
	    }
	}

	if (!data->isok() || (curl->putstream && !curl->putstream->isok()))
	{
	    if (curl->putstream && data->isok())
		data->close();

	    line = get_important_line(60000);
	    if (!line)
	    {
		doneurl();
		return;
	    }

	    if (strncmp(line, "226", 3))
		log("Unexpected message: %s\n", line);

	    if (curl->is_dir)
	    {
		if (curl->outstream)
		    curl->outstream->write("</table><hr/></body>\n"
					   "</html>\n");
		write("CWD /\r\n");
		log(WvLog::Debug5, "Waiting for response to CWD /\n");
		line = get_important_line(60000);
		if (!line)
		    return;
		
		if (strncmp(line, "250", 3))
		    log("Strange resonse to \"CWD /\": %s\n", line);
                    // Don't bother failing here.
	    }

	    doneurl();
	}
    }
}


WvString WvFtpStream::parse_for_links(char *line)
{
    WvString output_line("");
    trim_string(line);

    if (curl->is_dir && curl->outstream)
    {
	struct ftpparse fp;
	int res = ftpparse(&fp, line, strlen(line));
	if (res)
	{
	    char linkname[fp.namelen+1];
	    int i;
	    for (i = 0; i < fp.namelen; i++)
	    {
		if (fp.name[i] >= 32)
		    linkname[i] = fp.name[i];
		else
		{
		    linkname[i] = '?';
		}
	    }
	    linkname[i] = 0;

	    WvString linkurl(curl->url);
	    if (linkurl.cstr()[linkurl.len()-1] != '/')
		linkurl.append("/");
	    linkurl.append(linkname);
	    WvUrlLink *link = new WvUrlLink(linkname, linkurl);
	    curl->outstream->links.append(link, true);

	    output_line.append("<tr>\n");

	    output_line.append(WvString(" <td>%s%s</td>\n", linkname,
					fp.flagtrycwd ? "/" : ""));

	    if (fp.flagtryretr)
	    {
		if (!fp.sizetype)
		    output_line.append(" <td>? bytes</td>\n");
		else
		    output_line.append(WvString(" <td>%s bytes</td>\n",
						fp.size));
		if (fp.mtimetype > 0)
		    output_line.append(WvString(" <td>%s</td>\n", (fp.mtime)));
		else
		    output_line.append(" <td>?</td>\n");
	    }
	    else
		output_line.append(" <td></td>\n");

	    output_line.append("</tr>\n");
	}
    }
    return output_line;
}


WvIPPortAddr *WvFtpStream::parse_pasv_response(char *line)
{
    if (strncmp(line, "227 ", 4))
    {
	log("Strange response to PASV command: %s\n", line);
	seterr("strange response to PASV command");
	return NULL;
    }

    char *p = &line[3];
    while (!isdigit(*p))
    {
	if (*p == '\0' || *p == '\r' || *p == '\n')
	{
	    log("Couldn't parse PASV response: %s\n", line);
	    seterr("couldn't parse response to PASV command");
	    return NULL;
	}
	p++;
    }
    char *ipstart = p;

    for (int i = 0; i < 4; i++)
    {
	p = strchr(p, ',');
	if (!p)
	{
	    log("Couldn't parse PASV IP: %s\n", line);
	    seterr("couldn't parse PASV IP");
	    return NULL;
	}
	*p = '.';
    }
    *p = '\0';
    WvString pasvip(ipstart);
    p++;
    int pasvport;
    pasvport = atoi(p)*256;
    p = strchr(p, ',');
    if (!p)
    {
	log("Couldn't parse PASV IP port: %s\n", line);
	seterr("couldn't parse PASV IP port");
	return NULL;
    }
    pasvport += atoi(++p);

    WvIPPortAddr *res = new WvIPPortAddr(pasvip.cstr(), pasvport);

    return res;
}


WvHttpPool::WvHttpPool() : log("HTTP Pool", WvLog::Debug), conns(10),
				pipeline_incompatible(50)
{
    log("Pool initializing.\n");
    num_streams_created = 0;
}


WvHttpPool::~WvHttpPool()
{
    log("Created %s individual session%s during this run.\n",
	num_streams_created, num_streams_created == 1 ? "" : "s");
    if (geterr())
	log("Error was: %s\n", errstr());
    
    // these must get zapped before the URL list, since they have pointers
    // to URLs.
    zap();
    conns.zap();
}


bool WvHttpPool::pre_select(SelectInfo &si)
{
    bool sure = false;
    
    WvUrlStreamDict::Iter ci(conns);
    for (ci.rewind(); ci.next(); )
    {
//	if (!ci->isok() || urls.isempty())
	if (!ci->isok())
	{
	    unconnect(ci.ptr());
	    ci.rewind();
	    log(WvLog::Debug3, "Selecting true because of a dead stream.\n");
	    sure = true;
	}
    }
    
//    log(WvLog::Debug5, "pre_select: main:%s conns:%s urls:%s\n",
//         count(), conns.count(), urls.count());
    
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
            // nicely delete the url request
	    WvUrlStream::Target target(i->url.getaddr(), i->url.getuser());
            WvUrlStream *s = conns[target];
            if (s)
                s->delurl(i.ptr());
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

	WvUrlStream::Target target(i->url.getaddr(), i->url.getuser());

	//log(WvLog::Info, "remaddr is %s; username is %s\n", target.remaddr,
	//    target.username);
	s = conns[target];
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
	    if (!strncasecmp(i->url.getproto(), "http", 4))
		s = new WvHttpStream(target.remaddr,
				     i->url.getproto() == "https",
				     pipeline_incompatible);
	    else if (!strcasecmp(i->url.getproto(), "ftp"))
		s = new WvFtpStream(target.remaddr, target.username,
				    i->url.getpassword());
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
				    bool headers_only)
{
    log(WvLog::Debug4, "Adding a new url to pool: '%s'\n", _url);
    WvUrlRequest *url = new WvUrlRequest(_url, _headers, false, headers_only);
    urls.append(url, true);
    
    return url->outstream;
}


WvBufUrlStream *WvHttpPool::addputurl(WvStringParm _url,
				      WvStringParm _headers, WvStream *s,
				      bool create_dirs)
{
    log(WvLog::Debug4, "Adding a new put url to pool: '%s'\n", _url);
    WvUrlRequest *url = new WvUrlRequest(_url, _headers, s, create_dirs);
    urls.append(url, true);

    return url->outstream;
}


void WvHttpPool::unconnect(WvUrlStream *s)
{
    if (!s->target.username)
	log("Unconnecting stream to %s.\n", s->target.remaddr);
    else
	log("Unconnecting stream to %s@%s.\n", s->target.username,
	    s->target.remaddr);
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
	if (i->instream == s)
	    i->instream = NULL;
    }
    
    unlink(s);
    conns.remove(s);
}
