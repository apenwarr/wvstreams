/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Declarations for wvcon, wvin, wvout, and wverr global streams.
 */
#include "wvfdstream.h"
#include "wvmoniker.h"

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


static IWvStream *create_stdin(WvStringParm s, IObject *, void *)
{
    return new _WvConStream(0, -1);
}
static IWvStream *create_stdout(WvStringParm s, IObject *, void *)
{
    return new _WvConStream(-1, 1);
}
static IWvStream *create_stderr(WvStringParm s, IObject *, void *)
{
    return new _WvConStream(-1, 2);
}
static IWvStream *create_stdio(WvStringParm s, IObject *, void *)
{
    return new _WvConStream(0, 1);
}

static WvMoniker<IWvStream> reg0("stdin",  create_stdin);
static WvMoniker<IWvStream> reg1("stdout", create_stdout);
static WvMoniker<IWvStream> reg2("stderr", create_stderr);
static WvMoniker<IWvStream> reg3("stdio",  create_stdio);



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
    isopen = false;
    setfd(-1); // prevent WvFdStream from closing our fds
    WvFDStream::close();
}


bool _WvConStream::isok() const
{
    return isopen;
}


// console streams
#ifdef _WIN32

#include "../Win32WvStreams/streams/streams.h"
SocketFromFDMaker _zero(0, fd2socket_fwd);
SocketFromFDMaker _one(1, socket2fd_fwd);
SocketFromFDMaker _two(2, socket2fd_fwd);

static WvFDStream _wvcon(_zero.GetSocket(), _one.GetSocket());
static WvFDStream _wvin(_zero.GetSocket(), -1);
static WvFDStream _wvout(-1, _one.GetSocket());
static WvFDStream _wverr(-1, _two.GetSocket());

#else // _WIN32

static _WvConStream _wvcon(0, 1);
static _WvConStream _wvin(0, -1);
static _WvConStream _wvout(-1, 1);
static _WvConStream _wverr(-1, 2);

#endif // !_WIN32

WvStream *wvcon = &_wvcon;
WvStream *wvin = &_wvin;
WvStream *wvout = &_wvout;
WvStream *wverr = &_wverr;

