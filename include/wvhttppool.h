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

class WvBufUrlStream;
class WvUrlStream;
class WvHttpStream;


class WvUrlRequest
{
public:
    WvUrl url;
    WvString headers;
    WvUrlStream *instream;
    WvBufUrlStream *outstream;
    bool pipeline_test;
    bool headers_only;
    bool inuse;
    
    WvUrlRequest(WvStringParm _url, WvStringParm _headers,
		 bool _pipeline_test, bool _headers_only);
    ~WvUrlRequest();
    
    WvString request_str(bool keepalive);
    void done();
};

DeclareWvList(WvUrlRequest);


class WvBufUrlStream : public WvBufStream
{
public:
    WvString url;
    WvString proto;

    // HTTP stuff...
    WvString version;
    int status;
    WvHTTPHeaderDict headers; 

    WvBufUrlStream() : status(0), headers(10)
        {}
    virtual ~WvBufUrlStream()
        {}
};

DeclareWvTable(WvIPPortAddr);


// For looking up WvUrlStreams in a WvDict.
struct WvUrlStreamInfo
{
    WvIPPortAddr remaddr;
    WvString proto;

    bool operator== (const WvUrlStreamInfo &n2) const
    { return (proto == n2.proto && remaddr == n2.remaddr); }
};
unsigned WvHash(const WvUrlStreamInfo &n);


class WvUrlStream : public WvStreamClone
{
public:
    WvUrlStreamInfo info;
    static int max_requests;

protected:
    WvLog log;
    WvUrlRequestList urls, waiting_urls;
    int request_count;
    WvUrlRequest *curl; // current url
    virtual void doneurl() = 0;
    virtual void request_next() = 0;

public:
    WvUrlStream(const WvIPPortAddr &_remaddr, WvStringParm logname)
	: WvStreamClone(new WvTCPConn(_remaddr)), log(logname, WvLog::Debug)
    {}
    virtual ~WvUrlStream() {};

    virtual void close() = 0;
    void addurl(WvUrlRequest *url);
    
    virtual void execute() = 0;
};

DeclareWvDict(WvUrlStream, WvUrlStreamInfo, info);


class WvHttpStream : public WvUrlStream
{
public:
    static bool global_enable_pipelining;
    bool enable_pipelining;
    
private:
    int pipeline_test_count;
    bool ssl;
    WvIPPortAddrTable &pipeline_incompatible;
    WvString http_response, pipeline_test_response;
    
    enum { Unknown, Chunked, ContentLength, Infinity } encoding;
    size_t remaining;
    bool in_chunk_trailer, last_was_pipeline_test;

    virtual void doneurl();
    virtual void request_next();
    void start_pipeline_test(WvUrl *url);
    void send_request(WvUrlRequest *url, bool auto_free);
    void pipelining_is_broken(int why);
    
public:
    WvHttpStream(const WvIPPortAddr &_remaddr, bool ssl,
		 WvIPPortAddrTable &_pipeline_incompatible);
    virtual ~WvHttpStream();

    virtual void close();
    virtual void execute();
};


// FIXME: Rename this to WvUrlPool someday.
class WvHttpPool : public WvStreamList
{
    WvLog log;
    WvResolver dns;
    WvUrlStreamDict conns;
    WvUrlRequestList urls;
    int num_streams_created;
    
    WvIPPortAddrTable pipeline_incompatible;
    
public:
    WvHttpPool();
    virtual ~WvHttpPool();
    
    virtual bool pre_select(SelectInfo &si);
    virtual void execute();
    
    WvBufUrlStream *addurl(WvStringParm _url, WvStringParm _headers,
                            bool headers_only = false);
private:
    void unconnect(WvUrlStream *s);
    
public:
    bool idle() const 
        { return !urls.count(); }
};


#endif // __WVHTTPPOOL_H
