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
#include "strutils.h"

bool WvHttpStream::global_enable_pipelining = true;
int WvUrlStream::max_requests = 100;

unsigned WvHash(const WvUrlStream::Target &n)
{
    WvString key("%s%s", n.remaddr, n.username);
    return (WvHash(key));
}


WvUrlRequest::WvUrlRequest(WvStringParm _url, WvStringParm _method,
                WvStringParm _headers, WvStream *content_source,
                bool _create_dirs, bool _pipeline_test)
    : url(_url), headers(_headers)
{ 
    instream = NULL;
    create_dirs = _create_dirs;
    pipeline_test = _pipeline_test;
    method = _method;
    is_dir = false;    // for ftp primarily; set later

    if (pipeline_test)
    {
        outstream = NULL;
        putstream = NULL;
    }
    else
    {
        WvBufUrlStream *x = new WvBufUrlStream;
        outstream = x;
        x->death_notify = (WvStream **)&outstream;
        x->url = url;

        putstream = content_source;
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
        outstream->death_notify = NULL;
        outstream->seteof();
        outstream = NULL; 
    }
    if (putstream)
        putstream = NULL;
    inuse = false;
}


void WvUrlStream::addurl(WvUrlRequest *url)
{
    log(WvLog::Debug4, "Adding a new url: '%s'\n", url->url);

    assert(url->outstream);

    if (!url->url.isok())
        return;

    waiting_urls.append(url, false, "waiting_url");
    request_next();
}


void WvUrlStream::delurl(WvUrlRequest *url)
{
    log(WvLog::Debug4, "Removing an url: '%s'\n", url->url);

    if (url == curl)
        doneurl();
    waiting_urls.unlink(url);
    urls.unlink(url);
}


WvHttpPool::WvHttpPool() 
    : log("HTTP Pool", WvLog::Debug), conns(10),
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


void WvHttpPool::pre_select(SelectInfo &si)
{
    //    log(WvLog::Debug5, "pre_select: main:%s conns:%s urls:%s\n",
    //         count(), conns.count(), urls.count());

    WvIStreamList::pre_select(si);

    WvUrlStreamDict::Iter ci(conns);
    for (ci.rewind(); ci.next(); )
    {
        if (!ci->isok())
            si.msec_timeout = 0;
    }
    
    WvUrlRequestList::Iter i(urls);
    for (i.rewind(); i.next(); )
    {
        if (!i->instream)
        {
            log(WvLog::Debug4, "Checking dns for '%s'\n", i->url.gethost());
            if (i->url.resolve())
                si.msec_timeout = 0;
            else
                dns.pre_select(i->url.gethost(), si);    
        }
    }
}


bool WvHttpPool::post_select(SelectInfo &si)
{
    bool sure = false;

    WvUrlStreamDict::Iter ci(conns);
    for (ci.rewind(); ci.next(); )
    {
        if (!ci->isok())
        {
            log(WvLog::Debug4, "Selecting true because of a dead stream.\n");
            unconnect(ci.ptr());
            ci.rewind();
            sure = true;
        }
    }

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
            if (i->url.resolve() || dns.post_select(i->url.gethost(), si))
            {
                log(WvLog::Debug4, "Selecting true because of '%s'\n", i->url);
                sure = true;
            }
        }
    }

    return WvIStreamList::post_select(si) || sure;
}


void WvHttpPool::execute()
{
    WvIStreamList::execute();

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
                s = new WvHttpStream(target.remaddr, target.username,
                        i->url.getproto() == "https",
                        pipeline_incompatible);
            else if (!strcasecmp(i->url.getproto(), "ftp"))
                s = new WvFtpStream(target.remaddr, target.username,
                        i->url.getpassword());
            conns.add(s, true);

            // add it to the streamlist, so it can do things
            append(s, false, "http/ftp stream");
        }

        if (!i->instream)
        {
            s->addurl(i.ptr());
            i->instream = s;
        }
    }
}


WvBufUrlStream *WvHttpPool::addurl(WvStringParm _url, WvStringParm _method,
        WvStringParm _headers, WvStream *content_source, bool create_dirs)
{
    log(WvLog::Debug4, "Adding a new url to pool: '%s'\n", _url);
    WvUrlRequest *url = new WvUrlRequest(_url, _method, _headers, content_source,
                                         create_dirs, false);
    urls.append(url, true, "addurl");

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
