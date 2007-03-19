/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2007 Net Integration Technologies, Inc. and others.
 *
 */ 
#ifndef __WVHTTP_H
#define __WVHTTP_H

#include "wvurl.h"
#include "wvtcp.h"
#include "wvstreamclone.h"
#include "wvresolver.h"
#include "wvhashtable.h"


struct WvHTTPHeader
{
    WvString name, value;
    
    WvHTTPHeader(WvStringParm _name, WvStringParm _value)
	: name(_name), value(_value) 
    		{}
};


DeclareWvDict(WvHTTPHeader, WvString, name);

class WvSSLStream;

/** 
 * WvHTTPStream connects to an HTTP server and allows the requested file
 * to be retrieved using the usual WvStream-style calls.
 */
class WvHTTPStream : public WvStreamClone
{
public:
    WvHTTPStream(const WvURL &_url);
    virtual void execute();
    virtual size_t uread(void *buf, size_t count);

private:
    WvURL url;
    WvStream *conn;  // Can't use 'cloned' since it has no getline
    enum State {Connecting=0, ReadHeader1, ReadHeader, ReadData,
    		Done} state;
    WvHTTPHeaderDict headers;
    WvHTTPHeaderDict client_headers;
    size_t num_received;

public:
    const char *wstype() const { return "WvHTTPStream"; }
};

#endif // __WVHTTP_H
