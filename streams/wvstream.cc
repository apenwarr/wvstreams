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

// enable this to add some read/write trace messages (this can be VERY
// verbose)
#if 0
# define TRACE(x, y...) fprintf(stderr, x, ## y)
#else
# define TRACE(x, y...)
#endif

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
    select_ignores_buffer = outbuf_delayed_flush = false;
    queue_min = 0;
    autoclose_time = 0;
}


WvStream::~WvStream()
{
    close();
}


void WvStream::close()
{
    flush(2000);
    if (fd >= 0) ::close(fd);
    fd = -1;
}


void WvStream::autoforward_callback(WvStream &s, void *userdata)
{
    WvStream &s2 = *(WvStream *)userdata;
    char buf[1024];
    size_t len;
    
    while (s.isok() && s.select(0))
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
	bufu = uread(buf, count);
    else
    {
	// otherwise just read from the buffer
	if (bufu > count)
	    bufu = count;
    
	memcpy(buf, inbuf.get(bufu), bufu);
    }
    
    TRACE("read  obj 0x%08x, bytes %d/%d\n", (unsigned int)this, bufu, count);
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


size_t WvStream::write(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;

    
    size_t wrote = 0;
    
    // FIXME - re-enable this once the other tests are complete
    if (!outbuf_delayed_flush && outbuf.used())
	flush(0);
    
    if (!outbuf_delayed_flush && !outbuf.used())
	wrote = uwrite(buf, count);
    
    outbuf.put((unsigned char *)buf + wrote, count - wrote);
    TRACE("queue obj 0x%08x, bytes %d/%d, total %d\n", (unsigned int)this, count - wrote, count, outbuf.used());
    
    return count;
}


size_t WvStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int out = ::write(getfd(), buf, count);
    
    if (out < 0 && (errno == ENOBUFS || errno==EAGAIN))
	return 0; // kernel buffer full - data not written
    
    if (out < 0 || (count && out==0))
    {
	seterr(out < 0 ? errno : 0); // a more critical error
	return 0;
    }
    
    TRACE("write obj 0x%08x, bytes %d/%d\n", (unsigned int)this, out, count);
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


void WvStream::flush(time_t msec_timeout)
{
    size_t attempt, real;
    
    TRACE("flush obj 0x%08x, time %ld, outbuf length %d\n", (unsigned int)this, msec_timeout, outbuf.used());
    
    if (!isok()) return;
    
    while (outbuf.used())
    {
	attempt = outbuf.used();
	if (attempt > 1400)
	    attempt = 1400;
	real = uwrite(outbuf.get(attempt), attempt);
	if (real < attempt)
	    outbuf.unget(attempt - real);
	
	// since test_set() can call us, and select() calls test_set(),
	// we need to be careful not to call select() if it is unnecessary!
	if (!msec_timeout || !select(msec_timeout, false, true))
	    break;
    }

    if (autoclose_time)
    {
	time_t now = time(NULL);
	TRACE("Autoclose enabled for 0x%08X - now-time=%ld, buf %d bytes\n", 
		(unsigned int)this, now - autoclose_time, outbuf.used());
	if (!outbuf.used() || now > autoclose_time)
	{
	    autoclose_time = 0; // avoid infinite recursion!
	    close();
	}
    }
}


void WvStream::flush_then_close(int msec_timeout)
{
    time_t now = time(NULL);
    autoclose_time = now + (msec_timeout + 999) / 1000;
    
    TRACE("Autoclose SETUP for 0x%08X - buf %d bytes, timeout %ld sec\n", 
	    (unsigned int)this, outbuf.used(), autoclose_time - now);

    // as a fast track, we _could_ close here: but that's not a good idea,
    // since flush_then_close() deals with obscure situations, and we don't
    // want the caller to use it incorrectly.  So we make things _always_
    // break when the caller forgets to call select() later.
    
    flush(0);
}


bool WvStream::select_setup(SelectInfo &si)
{
    int fd;
    if (si.readable && !select_ignores_buffer && inbuf.used()
	  && inbuf.used() >= queue_min )
	return true; // already ready
    
    fd = getfd();
    if (fd < 0) return false;
    
    if (si.readable)
	FD_SET(fd, &si.read);
    if (si.writable || outbuf.used() || autoclose_time)
	FD_SET(fd, &si.write);
    if (si.isexception)
	FD_SET(fd, &si.except);
    
    if (si.max_fd < fd)
	si.max_fd = fd;
    
    return false;
}


bool WvStream::test_set(SelectInfo &si)
{
    size_t outbuf_used = outbuf.used();
    
    // flush the output buffer if possible
    if (getfd() >= 0 
	&& (outbuf_used || autoclose_time)
	&& FD_ISSET(getfd(), &si.write))
    {
	flush(0);
    }
    
    return fd >= 0  // flush() might have closed the file!
	&&  (FD_ISSET(getfd(), &si.read)
	 || (FD_ISSET(getfd(), &si.write) && (!outbuf_used || si.writable))
	 ||  FD_ISSET(getfd(), &si.except));
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




