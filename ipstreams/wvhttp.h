/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
#ifndef __WVHTTP_H
#define __WVHTTP_H

#include "wvtcp.h"
#include "wvstreamclone.h"
#include "wvresolver.h"
#include "wvhashtable.h"

class WvURL
{
public:
    WvURL(const WvString &url);
    ~WvURL();
    
    bool isok() const
        { return port != 0 && (resolving || addr != NULL); }
    const WvString &errstr() const
        { return err; }
    bool resolve(); // dns-resolve the hostname (returns true if done)

    operator WvString () const;
    
    // ONLY valid if resolve() returns true!
    const WvIPPortAddr &getaddr() const
        { return *addr; }
    const WvString &getfile() const
        { return file; }
    
protected:
    WvString hostname;
    int port;
    bool resolving;
    WvResolver dns;
    WvIPPortAddr *addr;
    WvString file, err;
};


struct WvHTTPHeader
{
    WvString name, value;
    
    WvHTTPHeader(const WvString &_name, const WvString &_value)
	: name(_name), value(_value) 
    		{}
};


DeclareWvDict(WvHTTPHeader, WvString, name);


class WvHTTPStream : public WvStreamClone
{
public:
    enum State {Resolving, Connecting, ReadHeader1, ReadHeader, ReadData};
    WvHTTPHeaderDict headers;
    size_t num_received;

    // do not delete '_url' before you delete this stream!
    WvHTTPStream(WvURL &_url);
    ~WvHTTPStream();
    
    virtual bool isok() const;
    virtual int geterr() const;
    virtual const char *errstr() const;
    virtual bool select_setup(SelectInfo &si);
    virtual size_t uread(void *buf, size_t count);

private:
    WvURL &url;
    WvTCPConn *http;
    State state;
};

#endif // __WVHTTP_H
