/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Declarations for wvcon, wvin, wvout, and wverr global streams.
 */
#include "wvfdstream.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(WvConStream);

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


static IWvStream *create_stdin(WvStringParm s)
{
    return new _WvConStream(0, -1);
}
static IWvStream *create_stdout(WvStringParm s)
{
    return new _WvConStream(-1, 1);
}
static IWvStream *create_stderr(WvStringParm s)
{
    return new _WvConStream(-1, 2);
}
static IWvStream *create_stdio(WvStringParm s)
{
    return new _WvConStream(0, 1);
}

static const UUID uuid0 = {0x1d51a28f, 0x2c8b, 0x4a08,
			  {0x8d, 0xf9, 0x13, 0x4f, 0x36, 0xfe, 0x9f, 0xc4}};
static WvMoniker<IWvStream> reg0("stdin",  uuid0, create_stdin);

static const UUID uuid1 = {0x270e4a5b, 0x5611, 0x41c0,
			   {0x81, 0xef, 0x47, 0x52, 0x3a, 0x41, 0x8c, 0x78}};
static WvMoniker<IWvStream> reg1("stdout", uuid1, create_stdout);

static const UUID uuid2 = {0x4924aa4b, 0x2f8b, 0x4eab,
			   {0xb3, 0xa9, 0x53, 0x1a, 0xbc, 0x29, 0x8e, 0xa7}};
static WvMoniker<IWvStream> reg2("stderr", uuid2, create_stderr);

static const UUID uuid3 = {0x0c3a926e, 0x4f96, 0x4a1c,
			   {0xad, 0xe9, 0xc0, 0x66, 0x2f, 0xa4, 0x8b, 0xa1}};
static WvMoniker<IWvStream> reg3("stdio",  uuid3, create_stdio);



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

#include "streams.h"
SocketFromFDMaker _zero(_dup(0), fd2socket_fwd, false);
SocketFromFDMaker _one(1, socket2fd_fwd, true);
SocketFromFDMaker _two(2, socket2fd_fwd, true);

static _WvConStream _wvcon(_zero.GetSocket(), _one.GetSocket());
static _WvConStream _wvin(_zero.GetSocket(), -1);
static _WvConStream _wvout(-1, _one.GetSocket());
static _WvConStream _wverr(-1, _two.GetSocket());

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

