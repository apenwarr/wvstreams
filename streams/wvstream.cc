/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Unified support for streams, that is, sequences of bytes that may or
 * may not be ready for read/write at any given time.
 * 
 * We provide typical read and write routines, as well as a select() function
 * for each stream.
 */
#include "wvstream.h"
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>


void WvStream::init()
{
    callfunc = NULL;
    userdata = NULL;
    errnum = 0;
    select_ignores_buffer = false;
    queue_min = 0;
}


WvStream::~WvStream()
{
    close();
}


void WvStream::close()
{
    if (fd >= 0) ::close(fd);
    fd = -1;
}


void WvStream::autoforward_callback(WvStream &s, void *userdata)
{
    WvStream &s2 = *(WvStream *)userdata;
    char buf[1024];
    size_t len;
    
    while (s.isok() && s2.isok() && s.select(0))
    {
	len = s.read(buf, sizeof(buf));
	s2.write(buf, len);
    }
}


void WvStream::execute()
{
    // do nothing by default
}


int WvStream::getfd() const
{
    return fd;
}


bool WvStream::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    return (FD_ISSET(getfd(), &r)
	    || FD_ISSET(getfd(), &w)
	    || FD_ISSET(getfd(), &x));
}


bool WvStream::isok() const
{
    return (fd != -1);
}


void WvStream::seterr(int _errnum)
{
    if (!errnum)
	errnum = _errnum;
    close();
}


void WvStream::seterr(const WvString &specialerr)
{
    if (!errnum)
    {
	errstring = specialerr;
	errnum = -1;
    }
    close();
}


int WvStream::geterr() const
{
    return errnum;
}


const char *WvStream::errstr() const
{
    if (errnum == -1)
    {
	assert(errstring.str);
	return errstring.str;
    }
    else
	return strerror(errnum);
}


size_t WvStream::read(void *buf, size_t count)
{
    size_t bufu = inbuf.used(), i;
    unsigned char *newbuf;

    bufu = inbuf.used();
    if (bufu < queue_min)
    {
	newbuf = inbuf.alloc(queue_min - bufu);
	i = uread(newbuf, queue_min - bufu);
	inbuf.unalloc(queue_min - bufu - i);
	
	bufu = inbuf.used();
    }
    
    if (bufu < queue_min)
	return 0;
        
    // if buffer is empty, do a hard read
    if (!bufu)
	return uread(buf, count);

    // otherwise just read from the buffer
    if (bufu > count)
	bufu = count;
    
    memcpy(buf, inbuf.get(bufu), bufu);
    return bufu;
}

size_t WvStream::uread(void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int in = ::read(getfd(), buf, count);
    
    if (in < 0 && (errno==EINTR || errno==EAGAIN || errno==ENOBUFS))
	return 0; // interrupted

    if (in < 0 || (count && in==0))
    {
	seterr(in < 0 ? errno : 0);
	return 0;
    }

    return in;
}


size_t WvStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    // usually people ignore the return value of write(), so we make
    // a feeble attempt to continue even if interrupted.
    int out;
    do
	out = ::write(getfd(), buf, count);
    while (out < 0 && (errno==EINTR || errno==EAGAIN));
    
    if (errno == ENOBUFS) // buffer full - data not written
	return 0;
    
    if (out < 0 || (count && out==0))
    {
	seterr(out < 0 ? errno : 0);
	return 0;
    }
    return out;
}


// search for a particular character in a buffer.  Like strchr, but ignores
// NUL bytes.
inline static char *find_char(unsigned char *buf, size_t count, char c)
{
    while (count-- > 0)
    {
	if (*(buf++) == c)
	    return (char *)(buf-1);
    }
    return NULL;
}


// NOTE:  wait_msec is implemented wrong, but it has cleaner code this way
// and can at least handle wait vs wait forever vs wait never.
char *WvStream::getline(time_t wait_msec, char separator)
{
    size_t i;
    unsigned char *buf;
    
    // if we get here, we either want to wait a bit or there is data
    // available.
    while (isok())
    {
	queuemin(0);
    
	// if there is a newline already, return its string.
	if ((i = inbuf.strchr(separator)) > 0)
	{
	    buf = inbuf.get(i);
	    buf[i-1] = 0;
	    return (char *)buf;
	}
	else if (!isok())    // uh oh, stream is in trouble.
	{
	    if (inbuf.used())
	    {
		// handle "EOF without newline" condition
		inbuf.alloc(1)[0] = 0; // null-terminate it
		return (char *)inbuf.get(inbuf.used());
	    }
	    else
		return NULL;   // nothing else to do!
	}

	if (wait_msec >= 0)
	{
	    // make select not return true until more data is available
	    if (inbuf.used())
		queuemin(inbuf.used() + 1);
	    
	    if (!select(wait_msec) && isok())
		return NULL;
	}

	// read a few bytes
	buf = inbuf.alloc(100);
	i = uread(buf, 100);
	inbuf.unalloc(100 - i);
    }
    
    // we timed out or had a socket error
    return NULL;
}


void WvStream::drain()
{
    char buf[1024];
    while (select(0))
	read(buf, sizeof(buf));
}


bool WvStream::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			    bool readable, bool writable, bool isexception)
{
    int fd;
    if (readable && !select_ignores_buffer && inbuf.used()
	  && inbuf.used() >= queue_min )
	return true; // already ready
    
    fd = getfd();
    if (fd < 0) return false;
    
    if (readable)    FD_SET(fd, &r);
    if (writable)    FD_SET(fd, &w);
    if (isexception) FD_SET(fd, &x);
    if (max_fd < fd) max_fd = fd;
    
    return false;
}


bool WvStream::select(time_t msec_timeout,
		      bool readable, bool writable, bool isexcept)
{
    int max_fd, sel;
    fd_set r, w, x;
    timeval tv;
    
    if (!readable && !writable && !isexcept)
	return false;  // why are you asking ME?
    
    if (!isok()) return false;

    max_fd = -1;
    FD_ZERO(&r);
    FD_ZERO(&w);
    FD_ZERO(&x);
    if (select_setup(r, w, x, max_fd, readable, writable, isexcept))
	return true;
    
    tv.tv_sec = msec_timeout / 1000;
    tv.tv_usec = (msec_timeout % 1000) * 1000;
    
    sel = ::select(max_fd+1, &r, &w, &x,
		   msec_timeout >= 0 ? &tv : (timeval*)NULL);

    if (sel < 0)
    {
	if (errno!=EAGAIN && errno!=EINTR && errno!=ENOBUFS)
	    seterr(errno);
	return false;
    }

    if (!sel)
	return false;	// timed out
    
    return isok() && test_set(r, w, x);
}


const WvAddr *WvStream::src() const
{
    return NULL;
}




bool WvFile::open(const WvString &filename, int mode, int create_mode)
{
    if (fd >= 0)
	close();
    fd = ::open(filename.str, mode | O_NONBLOCK, create_mode);
    if (fd < 0)
    {
	seterr(errno);
	return false;
    }
    else
	return true;
}
