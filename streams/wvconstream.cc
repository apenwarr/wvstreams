/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Declarations for wvcon, wvin, wvout, and wverr global streams.
 */
#include "wvfdstream.h"

// just like WvFDStream, but doesn't close the fd
class _WvConStream : public WvFDStream
{
public:
    bool isopen;
    
    _WvConStream(int _rfd, int _wfd);
    virtual ~_WvConStream();
    virtual void close();
    virtual bool isok() const;
};


_WvConStream::_WvConStream(int _rfd, int _wfd) : WvFDStream(_rfd, _wfd)
{
    isopen = true;
}


_WvConStream::~_WvConStream()
{
    close();
}


void _WvConStream::close()
{
    // skip over WvFDStream::close() - don't actually close the fds!
    isopen = false;
    WvStream::close();
}


bool _WvConStream::isok() const
{
    return isopen;
}


// console streams
static _WvConStream _wvcon(0, 1);
static _WvConStream _wvin(0, -1);
static _WvConStream _wvout(-1, 1);
static _WvConStream _wverr(-1, 2);

WvStream *wvcon = &_wvcon;
WvStream *wvin = &_wvin;
WvStream *wvout = &_wvout;
WvStream *wverr = &_wverr;

