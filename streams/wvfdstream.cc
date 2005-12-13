/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base class for streams built on Unix file descriptors.
 */
#include "wvfdstream.h"
#include "wvmoniker.h"
#include <fcntl.h>

#ifndef _WIN32
#include <sys/socket.h>

inline bool isselectable(int fd)
{
    return true;
}

#else // _WIN32

#define getsockopt(a,b,c,d,e) getsockopt(a,b,c,(char *)d, e) 
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define ENOBUFS WSAENOBUFS
#undef EAGAIN
#define EAGAIN WSAEWOULDBLOCK

#include "streams.h"

#undef errno
#define errno GetLastError()

// in win32, only sockets can be in the FD_SET for select()
static inline bool isselectable(int s)
{
    // if _get_osfhandle() works, it's a msvcrt fd, not a winsock handle.
    // msvcrt fds can't be select()ed on correctly.
    return ((HANDLE)_get_osfhandle(s) == INVALID_HANDLE_VALUE) 
	? true : false;
}

#endif // _WIN32


/***** WvFdStream *****/

static IWvStream *creator(WvStringParm s)
{
    return new WvFdStream(s.num());
}

static const UUID uuid = {0xf920f182, 0x1220, 0x4d69,
			  {0xa3, 0xfe, 0x74, 0xd5, 0x28, 0x28, 0xd6, 0x23}};
static WvMoniker<IWvStream> reg("fd", uuid, creator);

WvFdStream::WvFdStream(int _rwfd)
    : rfd(_rwfd), wfd(_rwfd)
{
    shutdown_read = shutdown_write = false;
}


WvFdStream::WvFdStream(int _rfd, int _wfd)
    : rfd(_rfd), wfd(_wfd)
{
    shutdown_read = shutdown_write = false;
}


WvFdStream::~WvFdStream()
{
    close();
}


static int _cloexec(int fd, bool close_on_exec)
{
#ifndef _WIN32 // there is no exec() in win32, so this is meaningless there
    return fcntl(fd, F_SETFD, close_on_exec ? FD_CLOEXEC : 0);
#else
    return 0;
#endif
}


static int _nonblock(int fd, bool nonblock)
{
#ifndef _WIN32
    int flag = fcntl(fd, F_GETFL);
    return fcntl(fd, F_SETFL,
		 (flag & ~O_NONBLOCK) | (nonblock ? O_NONBLOCK : 0));
#else
    u_long arg = nonblock ? 1 : 0;
    return ioctlsocket(fd, FIONBIO, &arg);
#endif    
}


void WvFdStream::set_nonblock(bool nonblock)
{
    int rfd = getrfd(), wfd = getwfd();
    if (rfd >= 0)
	_nonblock(rfd, nonblock);
    if (wfd >= 0 && rfd != wfd)
	_nonblock(wfd, nonblock);
}
    

void WvFdStream::set_close_on_exec(bool close_on_exec)
{
    int rfd = getrfd(), wfd = getwfd();
    if (rfd >= 0)
	_cloexec(rfd, close_on_exec);
    if (wfd >= 0 && rfd != wfd)
	_cloexec(wfd, close_on_exec);
}


void WvFdStream::close()
{
    // fprintf(stderr, "closing fdstream!\n");
    if (!closed)
    {
	WvStream::close();
	//fprintf(stderr, "closing%d:%d/%d\n", (int)this, rfd, wfd);
	if (rfd >= 0)
	    ::close(rfd);
	if (wfd >= 0 && wfd != rfd)
	    ::close(wfd);
	rfd = wfd = -1;
	//fprintf(stderr, "closed!\n");
    }
}


bool WvFdStream::isok() const
{
    return WvStream::isok() && (rfd != -1 || wfd != -1);
}


size_t WvFdStream::uread(void *buf, size_t count)
{
    assert(!count || buf);
    if (!count || !buf || !isok()) return 0;
    
    int in = ::read(rfd, buf, count);
    
    // a read that returns zero bytes signifies end-of-file (EOF).
    if (in <= 0)
    {
	if (in < 0 && (errno==EINTR || errno==EAGAIN || errno==ENOBUFS))
	    return 0; // interrupted

	seterr(in < 0 ? errno : 0);
	return 0;
    }

    // fprintf(stderr, "read %d bytes\n", in);
    return in;
}


size_t WvFdStream::uwrite(const void *buf, size_t count)
{
    assert(!count || buf);
    if (!buf || !count || !isok()) return 0;
    // fprintf(stderr, "write %d bytes\n", count);
    
    int out = ::write(wfd, buf, count);
    
    if (out <= 0)
    {
	int err = errno;
	// fprintf(stderr, "(fd%d-err-%d)", wfd, err);
	if (out < 0 && (err == ENOBUFS || err==EAGAIN))
	    return 0; // kernel buffer full - data not written (yet!)
    
	seterr(out < 0 ? err : 0); // a more critical error
	return 0;
    }

    //TRACE("write obj 0x%08x, bytes %d/%d\n", (unsigned int)this, out, count);
    return out;
}


void WvFdStream::maybe_autoclose()
{
    if (stop_write && !shutdown_write && !outbuf.used())
    {
	shutdown_write = true;
	if (wfd < 0)
	    return;
	if (rfd != wfd)
	    ::close(wfd);
	else
	    ::shutdown(wfd, SHUT_WR); // might be a socket        
	wfd = -1;
    }
    
    if (stop_read && !shutdown_read && !inbuf.used())
    {
	shutdown_read = true;
        if (rfd != wfd)
            ::close(rfd);
        else
            ::shutdown(rfd, SHUT_RD); // might be a socket
        rfd = -1;
    }
    
    WvStream::maybe_autoclose();
}


bool WvFdStream::pre_select(SelectInfo &si)
{
    bool result = WvStream::pre_select(si);
    
#if 0
    fprintf(stderr, "%d/%d wr:%d ww:%d wx:%d inh:%d\n", rfd, wfd,
	    si.wants.readable, si.wants.writable, si.wants.isexception,
	    si.inherit_request);
#endif
    if (isselectable(rfd))
    {
	if (si.wants.readable && (rfd >= 0))
	    FD_SET(rfd, &si.read);
    } 
    else
	result |= si.wants.readable;
    
    // FIXME: outbuf flushing should really be in WvStream::pre_select()
    // instead!  But it's hard to get the equivalent behaviour there.
    if (isselectable(wfd))
    {
	if ((si.wants.writable || outbuf.used() || autoclose_time) 
	      && (wfd >= 0))
	    FD_SET(wfd, &si.write);
    }
    else
	result |= si.wants.writable;
    
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


bool WvFdStream::post_select(SelectInfo &si)
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
      && read_requires_writable->isok()
      && !read_requires_writable->select(0, false, true))
	return result;
    if (val && si.wants.writable && write_requires_readable
      && write_requires_readable->isok()
      && !write_requires_readable->select(0, true, false))
	return result;
    return val || result;
}
