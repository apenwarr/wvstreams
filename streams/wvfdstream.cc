/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base class for streams built on Unix file descriptors.
 */
#include "wvfdstream.h"
#include "wvmoniker.h"

#ifndef _WIN32
#include <sys/socket.h>

#define isselectable(fd) (true)

#else // _WIN32

#define getsockopt(a,b,c,d,e) getsockopt(a,b,c,(char *)d, e) 
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define ENOBUFS WSAENOBUFS
#define EAGAIN WSAEWOULDBLOCK

// streams.cpp
int close(int fd);
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);

#undef errno
#define errno GetLastError()

// in win32, only sockets can be in the FD_SET for select()
inline bool isselectable(int s)
{
    static u_long crap;
    return (ioctlsocket(s, FIONREAD, &crap) == 0) ? true : (GetLastError() != WSAENOTSOCK);
}

#endif // _WIN32

/***** WvFDStream *****/

static IWvStream *creator(WvStringParm s, IObject *, void *)
{
    return new WvFDStream(s.num());
}

static WvMoniker<IWvStream> reg("fd", creator);

WvFDStream::WvFDStream(int _rwfd)
    : rfd(_rwfd), wfd(_rwfd)
{
}


WvFDStream::WvFDStream(int _rfd, int _wfd)
    : rfd(_rfd), wfd(_wfd)
{
}


WvFDStream::~WvFDStream()
{
    close();
}


void WvFDStream::close()
{
    WvStream::close();
    //fprintf(stderr, "closing:%d/%d\n", rfd, wfd);
    if (rfd >= 0)
	::close(rfd);
    if (wfd >= 0 && wfd != rfd)
	::close(wfd);
    rfd = wfd = -1;
}


bool WvFDStream::isok() const
{
    return WvStream::isok() && (rfd != -1 || wfd != -1);
}


size_t WvFDStream::uread(void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int in = ::read(rfd, buf, count);
    
    if (in < 0 && (errno==EINTR || errno==EAGAIN || errno==ENOBUFS))
	return 0; // interrupted

    // a read that returns zero bytes signifies end-of-file (EOF).
    if (in < 0 || (count && in==0))
    {
	seterr(in < 0 ? errno : 0);
	return 0;
    }

    return in;
}


size_t WvFDStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int out = ::write(wfd, buf, count);
    
    if (out < 0 && (errno == ENOBUFS || errno==EAGAIN))
	return 0; // kernel buffer full - data not written
    
    if (out < 0 || (count && out==0))
    {
	seterr(out < 0 ? errno : 0); // a more critical error
	return 0;
    }

    if (!outbuf.used() && want_nowrite && wfd < 0)
    {
        // copied from nowrite()
        if (rfd != wfd)
            ::close(wfd);
        else
            ::shutdown(rfd, SHUT_WR); // might be a socket

        want_nowrite = false;
        wfd = -1;
    }
    
    //TRACE("write obj 0x%08x, bytes %d/%d\n", (unsigned int)this, out, count);
    return out;
}


void WvFDStream::noread()
{
    if (rfd < 0)
        return;
    if (rfd != wfd)
        ::close(rfd);
    else
        ::shutdown(rfd, SHUT_RD); // might be a socket        
    rfd = -1;
}


void WvFDStream::nowrite()
{
    if (!outbuf.used())
    {
        if (rfd != wfd)
            ::close(wfd);
        else
            ::shutdown(rfd, SHUT_WR); // might be a socket

        want_nowrite = false;
        wfd = -1;
    }
    else
        WvStream::nowrite();
}


bool WvFDStream::pre_select(SelectInfo &si)
{
    bool result = WvStream::pre_select(si);
    
    if (isselectable(rfd))
    {
	if (si.wants.readable && (rfd >= 0))
	    FD_SET(rfd, &si.read);
    } else result |= si.wants.readable;
    if (isselectable(wfd))
    {
	if ((si.wants.writable || outbuf.used() || autoclose_time) && (wfd >= 0))
	    FD_SET(wfd, &si.write);
    } else result |= si.wants.writable ;
    
    if (si.wants.isexception)
    {
	if (rfd >= 0 && isselectable(rfd)) FD_SET(rfd, &si.except);
	if (wfd >= 0 && isselectable(wfd)) FD_SET(wfd, &si.except);
    }
    if (si.max_fd < rfd)
	si.max_fd = rfd;
    if (si.max_fd < wfd)
	si.max_fd = wfd;
    return result;
}


bool WvFDStream::post_select(SelectInfo &si)
{
    bool result = WvStream::post_select(si);
    
    // flush the output buffer if possible
    size_t outbuf_used = outbuf.used();
    if (wfd >= 0 && (outbuf_used || autoclose_time)
	&& FD_ISSET(wfd, &si.write) && should_flush())
    {
        flush_outbuf(0);
	
	// flush_outbuf() might have closed the file!
	if (!isok()) return result;
    }
    
    bool val = ((rfd >= 0 && FD_ISSET(rfd, &si.read)) ||
	    (wfd >= 0 && FD_ISSET(wfd, &si.write)) ||
	    (rfd >= 0 && FD_ISSET(rfd, &si.except)) ||
	    (wfd >= 0 && FD_ISSET(wfd, &si.except)));
    
    if (val && si.wants.readable && read_requires_writable
      && !read_requires_writable->select(0, false, true))
	return result;
    if (val && si.wants.writable && write_requires_readable
      && !write_requires_readable->select(0, true, false))
	return result;
    return val || result;
}
