/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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
    WvURL(WvStringParm url);
    WvURL(const WvURL &url);
    ~WvURL();
    
    bool isok() const
        { return port != 0 && (resolving || addr != NULL); }
    WvStringParm errstr() const
        { return err; }
    bool resolve(); // dns-resolve the hostname (returns true if done)

    operator WvString () const;
    
    // not actually defined - this just prevents accidental copying
    const WvURL &operator= (const WvURL &);
    
    // ONLY valid if resolve() returns true!
    const WvIPPortAddr &getaddr() const
        { return *addr; }
    WvStringParm getfile() const
        { return file; }
    WvStringParm gethost() const
        { return hostname; }
    int getport() const
        { return port; }
    
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
    
    WvHTTPHeader(WvStringParm _name, WvStringParm _value)
	: name(_name), value(_value) 
    		{ name.unique(); value.unique(); }
};


DeclareWvDict(WvHTTPHeader, WvString, name);

/** 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
class WvHTTPStream : public WvStreamClone
{
public:
    enum State {Resolving = 0, Connecting, ReadHeader1, ReadHeader, ReadData,
    		Done};
    WvHTTPHeaderDict headers;
    WvHTTPHeaderDict client_headers;
    size_t num_received;

    /**
     * Changed: now we copy _url in the constructor, so you can (and must)
     * delete it whenever you want.
     */
    WvHTTPStream(const WvURL &_url);
    ~WvHTTPStream();

    virtual bool isok() const;
    virtual int geterr() const;
    virtual const char *errstr() const;
    virtual bool pre_select(SelectInfo &si);
    virtual size_t uread(void *buf, size_t count);

public:
    WvURL url;
    WvTCPConn *http;
    State state;
};

#endif // __WVHTTP_H
