/* -*- Mode: C++ -*-
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
#include "wvlog.h"
#include "wvhashtable.h"
#include "wvhttp.h"
#include "wvbufstream.h"

class WvBufHttpStream;
class WvHttpStream;


class WvUrlRequest
{
public:
    WvUrl url;
    WvString headers;
    WvHttpStream *instream;
    WvBufHttpStream *outstream;
    bool pipeline_test;
    bool headers_only;
    
    WvUrlRequest(WvStringParm _url, WvStringParm _headers,
		 bool _pipeline_test, bool _headers_only);
    ~WvUrlRequest();
    
    WvString request_str(bool keepalive);
    void done();
};

DeclareWvList(WvUrlRequest);


class WvBufHttpStream : public WvBufStream
{
public:
    WvString url;
    WvString version;
    int status;
    WvHTTPHeaderDict headers; 

    WvBufHttpStream() : status(0), headers(10)
        {}
    virtual ~WvBufHttpStream()
        {}
};

DeclareWvTable(WvIPPortAddr);

class WvHttpStream : public WvStreamClone
{
public:
    WvIPPortAddr remaddr;
    
    static int max_requests;
    static bool global_enable_pipelining;
    bool enable_pipelining;
    
private:
    WvLog log;
    WvUrlRequestList urls, waiting_urls;
    int request_count, pipeline_test_count;
    WvIPPortAddrTable &pipeline_incompatible;
    WvString http_response, pipeline_test_response;
    
    WvUrlRequest *curl; // current url
    enum { Unknown, Chunked, ContentLength, Infinity } encoding;
    size_t remaining;
    bool in_chunk_trailer, last_was_pipeline_test;
    
    void doneurl();
    void start_pipeline_test(WvUrl *url);
    void send_request(WvUrlRequest *url, bool auto_free);
    void request_next();
    void pipelining_is_broken(int why);
    
public:
    WvHttpStream(const WvIPPortAddr &_remaddr, bool ssl,
		 WvIPPortAddrTable &_pipeline_incompatible);
    virtual ~WvHttpStream();
    virtual void close();
    
    void addurl(WvUrlRequest *url);
    
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
    
    WvIPPortAddrTable pipeline_incompatible;
    
public:
    WvHttpPool();
    virtual ~WvHttpPool();
    
    virtual bool pre_select(SelectInfo &si);
    virtual void execute();
    
    WvBufHttpStream *addurl(WvStringParm _url, WvStringParm _headers,
                            bool headers_only = false);
private:
    void unconnect(WvHttpStream *s);
    
public:
    bool idle() const 
        { return !urls.count(); }
};


#endif // __WVHTTPPOOL_H
