/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
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


WvStream::WvStream(int _fd)
{
    init();
    fd = _fd;
}

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


bool WvStream::test_set(SelectInfo &si)
{
    return fd >= 0 && (FD_ISSET(getfd(), &si.read)
	    || FD_ISSET(getfd(), &si.write)
	    || FD_ISSET(getfd(), &si.except));
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
	assert(errstring);
	return errstring;
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
    select(5000, false, true);
    int out;
    do
	out = ::write(getfd(), buf, count);
    while (out < 0 && errno==EINTR);
    
    if (out < 0 && (errno == ENOBUFS || errno==EAGAIN))
	return 0; // buffer full - data not written
    
    if (out < 0 || (count && out==0))
    {
	seterr(out < 0 ? errno : 0); // a more critical error
	return 0;
    }
    return out;
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


bool WvStream::select_setup(SelectInfo &si)
{
    int fd;
    if (si.readable && !select_ignores_buffer && inbuf.used()
	  && inbuf.used() >= queue_min )
	return true; // already ready
    
    fd = getfd();
    if (fd < 0) return false;
    
    if (si.readable)    FD_SET(fd, &si.read);
    if (si.writable)    FD_SET(fd, &si.write);
    if (si.isexception) FD_SET(fd, &si.except);
    if (si.max_fd < fd) si.max_fd = fd;
    
    return false;
}


bool WvStream::select(time_t msec_timeout,
		      bool readable, bool writable, bool isexcept)
{
    bool sure;
    int sel;
    timeval tv;
    SelectInfo si;
    
    if (!readable && !writable && !isexcept)
	return false;  // why are you asking ME?
    
    if (!isok()) return false;

    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
    si.readable = readable;
    si.writable = writable;
    si.isexception = isexcept;
    si.max_fd = -1;
    si.msec_timeout = msec_timeout;
    
    sure = select_setup(si);
    
    if (sure)
	tv.tv_sec = tv.tv_usec = 0; // never wait: already have a sure thing!
    else
    {
	tv.tv_sec = si.msec_timeout / 1000;
	tv.tv_usec = (si.msec_timeout % 1000) * 1000;
    }
    
    sel = ::select(si.max_fd+1, &si.read, &si.write, &si.except,
		   si.msec_timeout >= 0 ? &tv : (timeval*)NULL);
    
    if (sel < 0)
    {
	if (errno!=EAGAIN && errno!=EINTR && errno!=ENOBUFS)
	    seterr(errno);
	return sure;
    }

    if (!sel)
	return sure;	// timed out
    
    return isok() && test_set(si);
}


const WvAddr *WvStream::src() const
{
    return NULL;
}




bool WvFile::open(const WvString &filename, int mode, int create_mode)
{
    if (fd >= 0)
	close();
    fd = ::open(filename, mode | O_NONBLOCK, create_mode);
    if (fd < 0)
    {
	seterr(errno);
	return false;
    }

    fcntl(fd, F_SETFD, 1);
    return true;
}
