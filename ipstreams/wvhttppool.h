/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A fast, easy-to-use, parallelizing, pipelining HTTP/1.1 file retriever.
 * 
 * Just create a WvHttpPool object, add it to your list, and use pool.addurl()
 * to get a WvStream* that gives you the file you requested.
 */ 
#ifndef __WVHTTPPOOL_H
#define __WVHTTPPOOL_H

#include "wvurl.h"
#include "wvstreamlist.h"
#include "wvstreamclone.h"
#include "wvtcp.h"
#include "wvlog.h"
#include "wvhashtable.h"

class WvBufStream;
class WvHttpStream;


class WvUrlRequest
{
public:
    WvUrl url;
    WvString headers;
    WvHttpStream *instream;
    WvBufStream *outstream;
    
    WvUrlRequest(WvStringParm _url, WvStringParm _headers,
		 WvHttpStream *_instream);
    ~WvUrlRequest();
    
    WvString request_str(bool keepalive);
    void done();
};

DeclareWvList(WvUrlRequest);


class WvHttpStream : public WvStreamClone
{
    WvStream *cloned;
public:
    WvIPPortAddr remaddr;
    
    static bool enable_pipelining;
    static int max_requests;
    
private:
    WvLog log;
    WvTCPConn tcp;
    WvUrlRequestList urls, waiting_urls;
    int request_count;
    
    WvUrlRequest *curl; // current url
    size_t remaining;
    bool chunked, in_chunk_trailer;

public:
    WvHttpStream(const WvIPPortAddr &_remaddr);
    virtual ~WvHttpStream();
    virtual void close();
    
    void addurl(WvUrlRequest *url);
    void doneurl();
    void request_next();
    
    virtual void execute();
};

DeclareWvDict(WvHttpStream, WvIPPortAddr, remaddr);


class WvHttpPool : public WvStreamList
{
    WvLog log;
    WvResolver dns;
    WvHttpStreamDict conns;
    WvUrlRequestList urls;
    int num_streams_created;
    
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


#endif // __WVHTTPPOOL_H
