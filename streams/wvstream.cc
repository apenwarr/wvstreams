/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Unified support for streams, that is, sequences of bytes that may or
 * may not be ready for read/write at any given time.
 * 
 * We provide typical read and write routines, as well as a select() function
 * for each stream.
 */
#include <time.h>
#include <sys/types.h>
#include <assert.h>
#define __WVSTREAM_UNIT_TEST 1
#include "wvstream.h"
#include "wvtimeutils.h"
#include "wvcont.h"

#ifdef _WIN32
#define ENOBUFS WSAENOBUFS
#undef errno
#define errno GetLastError()
#ifdef __GNUC__
#include <sys/socket.h>
#endif
#include "streams.h"
#else
#include <errno.h>
#endif

// enable this to add some read/write trace messages (this can be VERY
// verbose)
#if 0
# ifndef _MSC_VER
#  define TRACE(x, y...) fprintf(stderr, x, ## y); fflush(stderr);
# else
#  define TRACE printf
# endif
#else
# ifndef _MSC_VER
#  define TRACE(x, y...)
# else
#  define TRACE
# endif
#endif

WvStream *WvStream::globalstream = NULL;

UUID_MAP_BEGIN(WvStream)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IWvStream)
  UUID_MAP_END


WvStream::WvStream():
    read_requires_writable(NULL),
    write_requires_readable(NULL),
    uses_continue_select(false),
    personal_stack_size(65536),
    alarm_was_ticking(false),
    stop_read(false),
    stop_write(false),
    closed(false),
    userdata(NULL),
    readcb(this, &WvStream::legacy_callback),
    max_outbuf_size(0),
    outbuf_delayed_flush(false),
    is_auto_flush(true),
    want_to_flush(true),
    is_flushing(false),
    queue_min(0),
    autoclose_time(0),
    alarm_time(wvtime_zero),
    last_alarm_check(wvtime_zero)
{
    TRACE("Creating wvstream %p\n", this);
    
#ifdef _WIN32
    WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2,0), &wsaData); 
    assert(result == 0);
#endif
}


// FIXME: interfaces (IWvStream) shouldn't have implementations!
IWvStream::IWvStream()
{
}


IWvStream::~IWvStream()
{
}


WvStream::~WvStream()
{
    TRACE("destroying %p\n", this);
    close();
    
    // if this assertion fails, then uses_continue_select is true, but you
    // didn't call terminate_continue_select() or close() before destroying
    // your object.  Shame on you!
    assert(!uses_continue_select || !call_ctx);
    
    call_ctx = 0; // finish running the suspended callback, if any
    TRACE("done destroying %p\n", this);
}


void WvStream::close()
{
    TRACE("flushing in wvstream...\n");
    flush(2000); // fixme: should not hardcode this stuff
    TRACE("(flushed)\n");

    closed = true;
    
    if (!!closecb)
    {
        IWvStreamCallback cb = closecb;
        closecb = 0; // ensure callback is only called once
        cb(*this);
    }
    
    // I would like to delete call_ctx here, but then if someone calls
    // close() from *inside* a continuable callback, we explode.  Oops!
    //call_ctx = 0; // destroy the context, if necessary
}


void WvStream::autoforward(WvStream &s)
{
    setcallback(autoforward_callback, &s);
    read_requires_writable = &s;
}


void WvStream::noautoforward()
{
    setcallback(0, NULL);
    read_requires_writable = NULL;
}


void WvStream::autoforward_callback(WvStream &s, void *userdata)
{
    WvStream &s2 = *(WvStream *)userdata;
    char buf[1024];
    size_t len;
    
    len = s.read(buf, sizeof(buf));
    // fprintf(stderr, "autoforward read %d bytes\n", (int)len);
    s2.write(buf, len);
}


void WvStream::_callback()
{
    execute();
    if (!! callfunc)
	callfunc(*this, userdata);
}


void *WvStream::_callwrap(void *)
{
    _callback();
    return NULL;
}


void WvStream::callback()
{
    TRACE("(?)");
    
    // if the alarm has gone off and we're calling callback... good!
    if (alarm_remaining() == 0)
    {
	alarm_time = wvtime_zero;
	alarm_was_ticking = true;
    }
    else
	alarm_was_ticking = false;
    
    assert(!uses_continue_select || personal_stack_size >= 1024);

#define TEST_CONTINUES_HARSHLY 0
#if TEST_CONTINUES_HARSHLY
#ifndef _WIN32
# warning "Using WvCont for *all* streams for testing!"
#endif
    if (1)
#else
    if (uses_continue_select && personal_stack_size >= 1024)
#endif
    {
	if (!call_ctx) // no context exists yet!
	{
	    call_ctx = WvCont(WvCallback<void*,void*>
			      (this, &WvStream::_callwrap),
			      personal_stack_size);
	}
	
	call_ctx(NULL);
    }
    else
	_callback();

    // if this assertion fails, a derived class's virtual execute() function
    // didn't call its parent's execute() function, and we didn't make it
    // all the way back up to WvStream::execute().  This doesn't always
    // matter right now, but it could lead to obscure bugs later, so we'll
    // enforce it.
}


bool WvStream::isok() const
{
    return !closed && WvErrorBase::isok();
}


void WvStream::seterr(int _errnum)
{
    if (!geterr()) // no pre-existing error
    {
        WvErrorBase::seterr(_errnum);
        close();
    }
}


size_t WvStream::read(WvBuf &outbuf, size_t count)
{
    // for now, just wrap the older read function
    size_t free = outbuf.free();
    if (count > free)
        count = free;

    WvDynBuf tmp;
    unsigned char *buf = tmp.alloc(count);
    size_t len = read(buf, count);
    tmp.unalloc(count - len);
    outbuf.merge(tmp);
    return len;
}


size_t WvStream::write(WvBuf &inbuf, size_t count)
{
    // for now, just wrap the older write function
    size_t avail = inbuf.used();
    if (count > avail)
        count = avail;
    const unsigned char *buf = inbuf.get(count);
    size_t len = write(buf, count);
    inbuf.unget(count - len);
    return len;
}


size_t WvStream::read(void *buf, size_t count)
{
    assert(!count || buf);
    
    size_t bufu, i;
    unsigned char *newbuf;

    bufu = inbuf.used();
    if (bufu < queue_min)
    {
	newbuf = inbuf.alloc(queue_min - bufu);
	assert(newbuf);
	i = uread(newbuf, queue_min - bufu);
	inbuf.unalloc(queue_min - bufu - i);
	
	bufu = inbuf.used();
    }
    
    if (bufu < queue_min)
    {
	maybe_autoclose();
	return 0;
    }
        
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
    maybe_autoclose();
    return bufu;
}


size_t WvStream::write(const void *buf, size_t count)
{
    assert(!count || buf);
    if (!isok() || !buf || !count || stop_write) return 0;
    
    size_t wrote = 0;
    if (!outbuf_delayed_flush && !outbuf.used())
    {
	wrote = uwrite(buf, count);
        count -= wrote;
        buf = (const unsigned char *)buf + wrote;
	// if (!count) return wrote; // short circuit if no buffering needed
    }
    if (max_outbuf_size != 0)
    {
        size_t canbuffer = max_outbuf_size - outbuf.used();
        if (count > canbuffer)
            count = canbuffer; // can't write the whole amount
    }
    if (count != 0)
    {
        outbuf.put(buf, count);
        wrote += count;
    }

    if (should_flush())
    {
        if (is_auto_flush)
            flush(0);
        else 
            flush_outbuf(0);
    }

    return wrote;
}


void WvStream::noread()
{
    stop_read = true;
    maybe_autoclose();
}


void WvStream::nowrite()
{
    stop_write = true;
    maybe_autoclose();
}


void WvStream::maybe_autoclose()
{
    if (stop_read && stop_write && !outbuf.used() && !inbuf.used() && !closed)
	close();
}


bool WvStream::isreadable()
{
    return isok() && select(0, true, false, false);
}


bool WvStream::iswritable()
{
    return !stop_write && isok() && select(0, false, true, false);
}


char *WvStream::blocking_getline(time_t wait_msec, int separator,
				 int readahead)
{
    // in fact, a separator of 0 would probably work fine.  Unfortunately,
    // the parameters of getline() changed recently to not include
    // wait_msec, so people keep trying to pass 0/-1 wait_msec in as the
    // separator.  Stop them now, before they get confused.
    // --mrwise - had to reenable 0 so that unit tests would pass when
    // merging, bug 11133
    //assert(separator != 0);
    assert(separator >= 0);
    assert(separator <= 255);
    
    //assert(uses_continue_select || wait_msec == 0);

    struct timeval timeout_time;
    if (wait_msec > 0)
        timeout_time = msecadd(wvtime(), wait_msec);
    
    maybe_autoclose();

    // if we get here, we either want to wait a bit or there is data
    // available.
    while (isok())
    {
	// fprintf(stderr, "(inbuf used = %d)\n", inbuf.used()); fflush(stderr);
        queuemin(0);
    
        // if there is a newline already, we have enough data.
        if (inbuf.strchr(separator) > 0)
	    break;
	else if (!isok() || stop_read)    // uh oh, stream is in trouble.
	    break;

        // make select not return true until more data is available
        queuemin(inbuf.used() + 1);

        // compute remaining timeout
        if (wait_msec > 0)
        {
            wait_msec = msecdiff(timeout_time, wvtime());
            if (wait_msec < 0)
                wait_msec = 0;
        }
	
	// FIXME: this is blocking_getline.  It shouldn't
	// call continue_select()!
        bool hasdata;
        if (wait_msec != 0 && uses_continue_select)
            hasdata = continue_select(wait_msec);
        else
            hasdata = select(wait_msec, true, false);
        
	if (!isok())
            break;

        if (hasdata)
        {
            // read a few bytes
	    WvDynBuf tmp;
            unsigned char *buf = tmp.alloc(readahead);
	    assert(buf);
            size_t len = uread(buf, readahead);
            tmp.unalloc(readahead - len);
	    inbuf.merge(tmp);
            hasdata = len > 0; // enough?
        }

	if (!isok())
	    break;
	
        if (!hasdata && wait_msec == 0)
	    return NULL; // handle timeout
    }
    if (!inbuf.used())
	return NULL;

    // return the appropriate data
    size_t i = 0;
    i = inbuf.strchr(separator);
    if (i > 0) {
	char *eol = (char *)inbuf.mutablepeek(i - 1, 1);
	assert(eol);
	*eol = 0;
	return const_cast<char*>((const char *)inbuf.get(i));
    } else {
	// handle "EOF without newline" condition
	// FIXME: it's very silly that buffers can't return editable
	// char* arrays.
	inbuf.alloc(1)[0] = 0; // null-terminate it
	return const_cast<char *>((const char *)inbuf.get(inbuf.used()));
    }
}


char *WvStream::continue_getline(time_t wait_msec, int separator,
				 int readahead)
{
    assert(false && "not implemented, come back later!");
    assert(uses_continue_select);
    return NULL;
}


void WvStream::drain()
{
    char buf[1024];
    while (isreadable())
	read(buf, sizeof(buf));
}


bool WvStream::flush(time_t msec_timeout)
{
    if (is_flushing) return false;
    
    TRACE("%p flush starts\n", this);

    is_flushing = true;
    want_to_flush = true;
    bool done = flush_internal(msec_timeout) // any other internal buffers
	&& flush_outbuf(msec_timeout);  // our own outbuf
    is_flushing = false;

    TRACE("flush stops (%d)\n", done);
    return done;
}


bool WvStream::should_flush()
{
    return want_to_flush;
}


bool WvStream::flush_outbuf(time_t msec_timeout)
{
    TRACE("%p flush_outbuf starts (isok=%d)\n", this, isok());
    bool outbuf_was_used = outbuf.used();
    
    // do-nothing shortcut for speed
    // FIXME: definitely makes a "measurable" difference...
    //   but is it worth the risk?
    if (!outbuf_was_used && !autoclose_time && !outbuf_delayed_flush)
    {
	maybe_autoclose();
	return true;
    }
    
    WvTime stoptime = msecadd(wvtime(), msec_timeout);
    
    // flush outbuf
    while (outbuf_was_used && isok())
    {
//	fprintf(stderr, "%p: fd:%d/%d, used:%d\n", 
//		this, getrfd(), getwfd(), outbuf.used());
	
	size_t attempt = outbuf.optgettable();
	size_t real = uwrite(outbuf.get(attempt), attempt);
	
	// WARNING: uwrite() may have messed up our outbuf!
	// This probably only happens if uwrite() closed the stream because
	// of an error, so we'll check isok().
	if (isok() && real < attempt)
	{
	    TRACE("flush_outbuf: unget %d-%d\n", attempt, real);
	    assert(outbuf.ungettable() >= attempt - real);
	    outbuf.unget(attempt - real);
	}
	
	// since post_select() can call us, and select() calls post_select(),
	// we need to be careful not to call select() if we don't need to!
	// post_select() will only call us with msec_timeout==0, and we don't
	// need to do select() in that case anyway.
	if (!msec_timeout)
	    break;
	if (msec_timeout >= 0 
	  && (stoptime < wvtime() || !select(msec_timeout, false, true)))
	    break;
	
	outbuf_was_used = outbuf.used();
    }

    // handle autoclose
    if (autoclose_time && isok())
    {
	time_t now = time(NULL);
	TRACE("Autoclose enabled for 0x%p - now-time=%ld, buf %d bytes\n", 
	      this, now - autoclose_time, outbuf.used());
	if ((flush_internal(0) && !outbuf.used()) || now > autoclose_time)
	{
	    autoclose_time = 0; // avoid infinite recursion!
	    close();
	}
    }

    TRACE("flush_outbuf: after autoclose chunk\n");
    if (outbuf_delayed_flush && !outbuf_was_used)
        want_to_flush = false;
    
    TRACE("flush_outbuf: now isok=%d\n", isok());

    // if we can't flush the outbuf, at least empty it!
    if (outbuf_was_used && !isok())
	outbuf.zap();

    maybe_autoclose();
    TRACE("flush_outbuf stops\n");
    
    return !outbuf_was_used;
}


bool WvStream::flush_internal(time_t msec_timeout)
{
    // once outbuf emptied, that's it for most streams
    return true;
}


int WvStream::getrfd() const
{
    return -1;
}


int WvStream::getwfd() const
{
    return -1;
}


void WvStream::flush_then_close(int msec_timeout)
{
    time_t now = time(NULL);
    autoclose_time = now + (msec_timeout + 999) / 1000;
    
    TRACE("Autoclose SETUP for 0x%p - buf %d bytes, timeout %ld sec\n", 
	    this, outbuf.used(), autoclose_time - now);

    // as a fast track, we _could_ close here: but that's not a good idea,
    // since flush_then_close() deals with obscure situations, and we don't
    // want the caller to use it incorrectly.  So we make things _always_
    // break when the caller forgets to call select() later.
    
    flush(0);
}


bool WvStream::pre_select(SelectInfo &si)
{
    maybe_autoclose();
    
    time_t alarmleft = alarm_remaining();
    
    if (!si.inherit_request && alarmleft == 0)
	return true; // alarm has rung

    if (!si.inherit_request)
    {
	si.wants.readable |= readcb;
	si.wants.writable |= writecb;
	si.wants.isexception |= exceptcb;
    }
    
    // handle read-ahead buffering
    if (si.wants.readable && inbuf.used() && inbuf.used() >= queue_min)
	return true; // already ready
    if (alarmleft >= 0
      && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft + 10;
    return false;
}


bool WvStream::post_select(SelectInfo &si)
{
    // FIXME: need sane buffer flush support for non FD-based streams
    // FIXME: need read_requires_writable and write_requires_readable
    //        support for non FD-based streams
    
    // note: flush(nonzero) might call select(), but flush(0) never does,
    // so this is safe.
    if (should_flush())
	flush(0);
    if (!si.inherit_request && alarm_remaining() == 0)
	return true; // alarm ticked
    return false;
}


bool WvStream::_build_selectinfo(SelectInfo &si, time_t msec_timeout,
    bool readable, bool writable, bool isexcept, bool forceable)
{
    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
    
    if (forceable)
    {
	si.wants.readable = readcb;
	si.wants.writable = writecb;
	si.wants.isexception = exceptcb;
    }
    else
    {
	si.wants.readable = readable;
	si.wants.writable = writable;
	si.wants.isexception = isexcept;
    }
    
    si.max_fd = -1;
    si.msec_timeout = msec_timeout;
    si.inherit_request = ! forceable;
    si.global_sure = false;

    if (!isok()) return false;

    bool sure = pre_select(si);
    if (globalstream && forceable && (globalstream != this))
    {
	WvStream *s = globalstream;
	globalstream = NULL; // prevent recursion
	si.global_sure = s->xpre_select(si, SelectRequest(false, false, false));
	globalstream = s;
    }
    if (sure || si.global_sure)
        si.msec_timeout = 0;
    return sure;
}


int WvStream::_do_select(SelectInfo &si)
{
    // prepare timeout
    timeval tv;
    tv.tv_sec = si.msec_timeout / 1000;
    tv.tv_usec = (si.msec_timeout % 1000) * 1000;
    
#ifdef _WIN32
    // selecting on an empty set of sockets doesn't cause a delay in win32.
    SOCKET fakefd = socket(PF_INET, SOCK_STREAM, 0);
    FD_SET(fakefd, &si.except);
#endif    
    
    // block
    int sel = ::select(si.max_fd+1, &si.read, &si.write, &si.except,
        si.msec_timeout >= 0 ? &tv : (timeval*)NULL);

    // handle errors.
    //   EAGAIN and EINTR don't matter because they're totally normal.
    //   ENOBUFS is hopefully transient.
    //   EBADF is kind of gross and might imply that something is wrong,
    //      but it happens sometimes...
    if (sel < 0 
      && errno != EAGAIN && errno != EINTR 
      && errno != EBADF
      && errno != ENOBUFS
      )
    {
        seterr(errno);
    }
#ifdef _WIN32
    ::close(fakefd);
#endif
    TRACE("select() returned %d\n", sel);
    return sel;
}


bool WvStream::_process_selectinfo(SelectInfo &si, bool forceable)
{
    if (!isok()) return false;
    
    bool sure = post_select(si);
    if (globalstream && forceable && (globalstream != this))
    {
	WvStream *s = globalstream;
	globalstream = NULL; // prevent recursion
	si.global_sure = s->xpost_select(si, SelectRequest(false, false, false))
					 || si.global_sure;
	globalstream = s;
    }
    return sure;
}


bool WvStream::_select(time_t msec_timeout,
    bool readable, bool writable, bool isexcept, bool forceable)
{
    SelectInfo si;
    bool sure = _build_selectinfo(si, msec_timeout,
				  readable, writable, isexcept, forceable);
    
    if (!isok())
	return false;
    
    // the eternal question: if 'sure' is true already, do we need to do the
    // rest of this stuff?  If we do, it might increase fairness a bit, but
    // it encourages select()ing when we know something fishy has happened -
    // when a stream is !isok() in a list, for example, pre_select() returns
    // true.  If that's the case, our SelectInfo structure might not be
    // quite right (eg. it might be selecting on invalid fds).  That doesn't
    // sound *too* bad, so let's go for the fairness.

    int sel = _do_select(si);
    if (sel >= 0)
        sure = _process_selectinfo(si, forceable) || sure; // note the order
    if (si.global_sure && globalstream && forceable && (globalstream != this))
	globalstream->callback();
    return sure;
}


IWvStream::SelectRequest WvStream::get_select_request()
{
    return IWvStream::SelectRequest(readcb, writecb, exceptcb);
}


void WvStream::force_select(bool readable, bool writable, bool isexception)
{
    if (readable)
	readcb = IWvStreamCallback(this, &WvStream::legacy_callback);
    if (writable)
	writecb = IWvStreamCallback(this, &WvStream::legacy_callback);
    if (isexception)
	exceptcb = IWvStreamCallback(this, &WvStream::legacy_callback);
}


void WvStream::undo_force_select(bool readable, bool writable, bool isexception)
{
    if (readable)
	readcb = 0;
    if (writable)
	writecb = 0;
    if (isexception)
	exceptcb = 0;
}


void WvStream::alarm(time_t msec_timeout)
{
    if (msec_timeout >= 0)
        alarm_time = msecadd(wvtime(), msec_timeout);
    else
	alarm_time = wvtime_zero;
}


time_t WvStream::alarm_remaining()
{
    if (alarm_time.tv_sec)
    {
	WvTime now = wvtime();

	// Time is going backward!
	if (now < last_alarm_check)
	{
#if 0 // okay, I give up.  Time just plain goes backwards on some systems.
	    // warn only if it's a "big" difference (sigh...)
	    if (msecdiff(last_alarm_check, now) > 200)
		fprintf(stderr, " ************* TIME WENT BACKWARDS! "
			"(%ld:%ld %ld:%ld)\n",
			last_alarm_check.tv_sec, last_alarm_check.tv_usec,
			now.tv_sec, now.tv_usec);
#endif
	    alarm_time = tvdiff(alarm_time, tvdiff(last_alarm_check, now));
	}

	last_alarm_check = now;

        time_t remaining = msecdiff(alarm_time, now);
        if (remaining < 0)
            remaining = 0;
        return remaining;
    }
    return -1;
}


bool WvStream::continue_select(time_t msec_timeout)
{
    assert(uses_continue_select);
    
    // if this assertion triggers, you probably tried to do continue_select()
    // while inside terminate_continue_select().
    assert(call_ctx);
    
    if (msec_timeout >= 0)
	alarm(msec_timeout);

    alarm(msec_timeout);
    WvCont::yield();
    alarm(-1); // cancel the still-pending alarm, or it might go off later!
    
    // when we get here, someone has jumped back into our task.
    // We have to select(0) here because it's possible that the alarm was 
    // ticking _and_ data was available.  This is aggravated especially if
    // msec_delay was zero.  Note that running select() here isn't
    // inefficient, because if the alarm was expired then pre_select()
    // returned true anyway and short-circuited the previous select().
    TRACE("hello-%p\n", this);
    return !alarm_was_ticking || select(0, readcb, writecb, exceptcb);
}


void WvStream::terminate_continue_select()
{
    close();
    call_ctx = 0; // destroy the context, if necessary
}


const WvAddr *WvStream::src() const
{
    return NULL;
}


void WvStream::setcallback(WvStreamCallback _callfunc, void *_userdata)
{ 
    callfunc = _callfunc;
    userdata = _userdata;
    call_ctx = 0; // delete any in-progress WvCont
}


void WvStream::legacy_callback(IWvStream& s)
{
    execute();
    if (!! callfunc)
	callfunc(*this, userdata);
}


IWvStreamCallback WvStream::setreadcallback(IWvStreamCallback _callback)
{
    IWvStreamCallback tmp = readcb;

    readcb = _callback;

    return tmp;
}


IWvStreamCallback WvStream::setwritecallback(IWvStreamCallback _callback)
{
    IWvStreamCallback tmp = writecb;

    writecb = _callback;

    return tmp;
}


IWvStreamCallback WvStream::setexceptcallback(IWvStreamCallback _callback)
{
    IWvStreamCallback tmp = exceptcb;

    exceptcb = _callback;

    return tmp;
}


IWvStreamCallback WvStream::setclosecallback(IWvStreamCallback _callback)
{
    IWvStreamCallback tmp = closecb;
    if (isok())
	closecb = _callback;
    else
    {
	// already closed?  notify immediately!
	closecb = 0;
	if (!!_callback)
	    _callback(*this);
    }
    return tmp;
}


void WvStream::unread(WvBuf &unreadbuf, size_t count)
{
    WvDynBuf tmp;
    tmp.merge(unreadbuf, count);
    tmp.merge(inbuf);
    inbuf.zap();
    inbuf.merge(tmp);
}
