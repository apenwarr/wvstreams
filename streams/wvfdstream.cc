/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base class for streams built on Unix file descriptors.
 */
#include "wvfdstream.h"
#include <sys/socket.h>

// console streams
static WvFDStream _wvcon(dup(0), dup(1));
static WvFDStream _wvin(dup(0), -1);
static WvFDStream _wvout(-1, dup(1));
static WvFDStream _wverr(-1, dup(2));

WvStream *wvcon = &_wvcon;
WvStream *wvin = &_wvin;
WvStream *wvout = &_wvout;
WvStream *wverr = &_wverr;

/***** WvFDStream *****/

WvFDStream::WvFDStream(int _rwfd) :
    rfd(_rwfd), wfd(_rwfd)
{
}


WvFDStream::WvFDStream(int _rfd, int _wfd) :
    rfd(_rfd), wfd(_wfd)
{
}


WvFDStream::~WvFDStream()
{
    close();
}


void WvFDStream::close()
{
    WvStream::close();
    if (rfd >= 0)
	::close(rfd);
    if (wfd >= 0 && wfd != rfd)
	::close(wfd);
    rfd = wfd = -1;
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
    if (wfd < 0)
        return;
    if (rfd != wfd)
        ::close(wfd);
    else
        ::shutdown(rfd, SHUT_WR); // might be a socket
    wfd = -1;
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
    
    //TRACE("write obj 0x%08x, bytes %d/%d\n", (unsigned int)this, out, count);
    return out;
}


bool WvFDStream::pre_select(SelectInfo &si)
{
    bool result = WvStream::pre_select(si);
    
    if (si.wants.readable && (rfd >= 0))
	FD_SET(rfd, &si.read);
    if ((si.wants.writable || outbuf.used() || autoclose_time) && (wfd >= 0))
	FD_SET(wfd, &si.write);
    if (si.wants.isexception)
    {
	if (rfd >= 0) FD_SET(rfd, &si.except);
	if (wfd >= 0) FD_SET(wfd, &si.except);
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
    if (wfd >= 0 
	&& (outbuf_used || autoclose_time)
	&& FD_ISSET(wfd, &si.write))
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
