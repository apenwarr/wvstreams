/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
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
    WvTCPConn *tcp;

    /**
     * Changed: now we copy _url in the constructor, so you can (and must)
     * delete it whenever you want.
     */
    WvHTTPStream(const WvURL &_url);

    virtual bool isok() const;
    virtual int geterr() const;
    virtual WvString errstr() const;
    virtual bool pre_select(SelectInfo &si);
    virtual size_t uread(void *buf, size_t count);

public:
    WvURL url;
    State state;
};

#endif // __WVHTTP_H
