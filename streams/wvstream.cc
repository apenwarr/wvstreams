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
#include "wvstream.h"
#include "wvtask.h"
#include "wvtimeutils.h"
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#ifdef _WIN32
#define ENOBUFS WSAENOBUFS
#undef errno
#define errno GetLastError()
class RunWinSockInitialize;
extern RunWinSockInitialize __runinitialize;
#else
#include <errno.h>
#endif

// enable this to add some read/write trace messages (this can be VERY
// verbose)
#if 0
# define TRACE(x, y...) fprintf(stderr, x, ## y); fflush(stderr);
#else
#ifndef _MSC_VER
# define TRACE(x, y...)
#else
# define TRACE
#endif
#endif

WvStream *WvStream::globalstream = NULL;

XUUID_MAP_BEGIN(IWvStream)
  XUUID_MAP_ENTRY(IObject)
  XUUID_MAP_ENTRY(IWvStream)
  XUUID_MAP_END

WvStream::WvStream()
{
#ifdef _WIN32
    void *addy = &__runinitialize; // this ensures WSAStartup() is run
#endif
    wvstream_execute_called = false;
    userdata = closecb_data = NULL;
    errnum = 0;
    max_outbuf_size = 0;
    outbuf_delayed_flush = false;
    want_to_flush = true;
    is_flushing = false;
    is_auto_flush = true;
    alarm_was_ticking = false;
    force.readable = true;
    force.writable = force.isexception = false;
    read_requires_writable = write_requires_readable = NULL;
    running_callback = false;
    want_nowrite = false;
    queue_min = 0;
    autoclose_time = 0;
    alarm_time = wvtime_zero;
    last_alarm_check = wvtime_zero;
    taskman = 0;
    
    // magic multitasking support
    uses_continue_select = false;
    personal_stack_size = 65536;
    task = NULL;
}


IWvStream::IWvStream()
{
}


IWvStream::~IWvStream()
{
}


WvStream::~WvStream()
{
    TRACE("destroying %p\n", this);
    if (running_callback)
    {
	// user should have called terminate_continue_select()...
	TRACE("eek! destroying while running_callback!\n");
	assert(!running_callback);
    }
    close();
    
    if (task)
    {
	while (task->isrunning())
	    taskman->run(*task);
	task->recycle();
	task = NULL;
    }
    TRACE("done destroying %p\n", this);
    if (taskman)
	taskman->unlink();
}


void WvStream::close()
{
    flush(2000); // fixme: should not hardcode this stuff
    if (!! closecb_func)
    {
        WvStreamCallback cb = closecb_func;
        closecb_func = 0; // ensure callback is only called once
        cb(*this, closecb_data);
    }
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
    s2.write(buf, len);
}


// this is run in the subtask owned by 'stream', if any; NOT necessarily
// the task that runs WvStream::callback().  That's why this needs to be
// a separate function.
void WvStream::_callback(void *stream)
{
    WvStream *s = (WvStream *)stream;
    
    s->running_callback = true;
    
    s->wvstream_execute_called = false;
    s->execute();
    if (!! s->callfunc)
	s->callfunc(*s, s->userdata);
    
    // if this assertion fails, a derived class's virtual execute() function
    // didn't call its parent's execute() function, and we didn't make it
    // all the way back up to WvStream::execute().  This doesn't always
    // matter right now, but it could lead to obscure bugs later, so we'll
    // enforce it.
    assert(s->wvstream_execute_called);
    
    s->running_callback = false;
}


void WvStream::callback()
{
    TRACE("(?)");
    
    // callback is already running -- don't try to start it again, or we
    // could end up in an infinite loop!
    if (running_callback)
	return;
    
    // if the alarm has gone off and we're calling callback... good!
    if (alarm_remaining() == 0)
    {
	alarm_time = wvtime_zero;
	alarm_was_ticking = true;
    }
    else
	alarm_was_ticking = false;
    
    assert(!uses_continue_select || personal_stack_size >= 1024);
    
//    if (1)
    if (uses_continue_select && personal_stack_size >= 1024)
    {
	if (!taskman)
	    taskman = WvTaskMan::get();
    
	if (!task)
	{
	    TRACE("(!)");
	    task = taskman->start("streamexec", _callback, this,
				  personal_stack_size);
	}
	else if (!task->isrunning())
	{
	    TRACE("(.)");
	    task->start("streamexec2", _callback, this);
	}
	
	// This loop is much more subtle than it looks.
	// By implementing it this way, we provide something that works
	// like a typical callback() stack: that is, a child callback
	// must return before the parent's callback does.
	// 
	// What _actually_ happens is a child will call yield() upon returning
	// from its callback function, which exits the taskman and returns to
	// the top level.  The top level, though, is running this loop, which
	// re-executes taskman->run() since its child (which is eventually
	// the parent of the child that called yield()) hasn't finished yet.
	// We build our way all the way back up to the first-level parent of
	// the child calling yield(), which now notices its child has finished
	// and continues on in its execute() function.
	// 
	// continue_select() will set running_callback to false, even though
	// it doesn't actually return from the callback function.  That
	// causes this loop to terminate, and the callback will get resumed
	// later when select() returns true.
	do
	{
	    taskman->run(*task);
	} while (task && task->isrunning() && running_callback);
    }
    else
	_callback(this);
    
    /* DON'T PUT ANY CODE HERE!
     * 
     * WvStreamList calls its child streams above via taskman->run().
     * If a child is deleted, it waits for its callback task to finish the
     * current iteration, then recycles its WvTask object and allows the
     * "delete" call to finish, so the object no longer exists.
     * 
     * The catch: the callback() function is actually running in
     * the WvStreamList's task (if any), which hasn't had a chance to
     * exit yet.  Next time we jump into the WvStreamList, we will arrive
     * immediately after the taskman->run() line, ie. right here in the
     * code.  In that case, the 'this' pointer could be pointing at an
     * invalid object, so we should just exit before we do something stupid.
     */
}


void WvStream::execute()
{
    // do nothing by default, but notice that we were here.
    wvstream_execute_called = true;
}


bool WvStream::isok() const
{
    return WvError::isok();
}


void WvStream::seterr(int _errnum)
{
    if (!errnum)
    {
        WvError::seterr(_errnum);
        close();
    }
}


size_t WvStream::read(WvBuf &outbuf, size_t count)
{
    // for now, just wrap the older read function
    size_t free = outbuf.free();
    if (count > free)
        count = free;
    unsigned char *buf = outbuf.alloc(count);
    size_t len = read(buf, count);
    outbuf.unalloc(count - len);
    return len;
}


size_t WvStream::continue_read(time_t wait_msec, WvBuf &outbuf, size_t count)
{
    // for now, just wrap the older read function
    size_t free = outbuf.free();
    if (count > free)
        count = free;
    unsigned char *buf = outbuf.alloc(count);
    size_t len = continue_read(wait_msec, buf, count);
    outbuf.unalloc(count - len);
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


size_t WvStream::continue_read(time_t wait_msec, void *buf, size_t count)
{
    assert(uses_continue_select);

    if (!count)
        return 0;

    if (wait_msec >= 0)
        alarm(wait_msec);

    queuemin(count);

    int got = 0;

    while (isok())
    {
        if (continue_select(-1))
        {
	    if ((got = read(buf, count)) != 0)
		break;
	    if (alarm_was_ticking) 
		break;
        }
    }

    if (wait_msec >= 0)
        alarm(-1);

    queuemin(0);
    
    return got;
}


size_t WvStream::write(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    size_t wrote = 0;
    if (!outbuf_delayed_flush && !outbuf.used())
    {
	wrote = uwrite(buf, count);
        count -= wrote;
        buf = (const unsigned char*)buf + wrote;
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


void WvStream::nowrite()
{
    if (getwfd() < 0)
        return;

    want_nowrite = true;
}


char *WvStream::getline(time_t wait_msec, char separator, int readahead)
{
    struct timeval timeout_time;
    if (wait_msec > 0)
        timeout_time = msecadd(wvtime(), wait_msec);

    // if we get here, we either want to wait a bit or there is data
    // available.
    while (isok())
    {
        queuemin(0);
    
        // if there is a newline already, return its string.
        size_t i = inbuf.strchr(separator);
        if (i > 0)
        {
	    char *eol = (char *)inbuf.mutablepeek(i - 1, 1);
	    assert(eol);
	    *eol = 0;
            return (char *)inbuf.get(i);
        }
        else if (!isok())    // uh oh, stream is in trouble.
        {
            if (inbuf.used())
            {
                // handle "EOF without newline" condition
		// FIXME: it's very silly that buffers can't return editable
		// char* arrays.
                inbuf.alloc(1)[0] = 0; // null-terminate it
                return const_cast<char*>(
                    (const char*)inbuf.get(inbuf.used()));
            }
            else
                break; // nothing else to do!
        }

        // make select not return true until more data is available
        size_t needed = inbuf.used() + 1;
        queuemin(needed);

        // compute remaining timeout
        if (wait_msec > 0)
        {
            wait_msec = msecdiff(timeout_time, wvtime());
            if (wait_msec < 0)
                wait_msec = 0;
        }
        
        bool hasdata;
        if (uses_continue_select)
            hasdata = continue_select(wait_msec);
        else
            hasdata = select(wait_msec, true, false);
        if (!isok())
            break;

        if (hasdata)
        {
            // read a few bytes
            unsigned char *buf = inbuf.alloc(readahead);
            size_t len = uread(buf, readahead);
            inbuf.unalloc(readahead - len);
            hasdata = inbuf.used() >= needed; // enough?
        }

        if (!hasdata && wait_msec == 0)
            break; // handle timeout
    }
    // we timed out or had a socket error
    return NULL;
}


void WvStream::drain()
{
    char buf[1024];
    while (select(0, true, false, false))
	read(buf, sizeof(buf));
}


bool WvStream::flush(time_t msec_timeout)
{
    if (is_flushing) return false;

    is_flushing = true;
    want_to_flush = true;
    bool done = flush_internal(msec_timeout) // any other internal buffers
	&& flush_outbuf(msec_timeout);  // our own outbuf
    is_flushing = false;
    return done;
}


bool WvStream::should_flush()
{
    return want_to_flush;
}


bool WvStream::flush_outbuf(time_t msec_timeout)
{
    // flush outbuf
    while (isok() && outbuf.used())
    {
	//fprintf(stderr, "%p: fd:%d/%d, used:%d\n", 
	//	this, getrfd(), getwfd(), outbuf.used());
	
	size_t attempt = outbuf.used();
	size_t real = uwrite(outbuf.get(attempt), attempt);
	if (real < attempt)
	    outbuf.unget(attempt - real);
	
	// since post_select() can call us, and select() calls post_select(),
	// we need to be careful not to call select() if we don't need to!
	if (!msec_timeout || !select(msec_timeout, false, true))
        {
            if (msec_timeout >= 0)
                break;
        }
    }

    // handle autoclose
    if (isok() && autoclose_time)
    {
	time_t now = time(NULL);
	TRACE("Autoclose enabled for 0x%08X - now-time=%ld, buf %d bytes\n", 
	      (unsigned int)this, now - autoclose_time, outbuf.used());
	if ((flush_internal(0) && !outbuf.used()) || now > autoclose_time)
	{
	    autoclose_time = 0; // avoid infinite recursion!
	    close();
	}
    }

    if (!outbuf.used() && outbuf_delayed_flush)
        want_to_flush = false;

    // if we can't flush the outbuf, at least empty it!
    if (!isok())
	outbuf.zap();

    return !outbuf.used();
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
    
    TRACE("Autoclose SETUP for 0x%08X - buf %d bytes, timeout %ld sec\n", 
	    (unsigned int)this, outbuf.used(), autoclose_time - now);

    // as a fast track, we _could_ close here: but that's not a good idea,
    // since flush_then_close() deals with obscure situations, and we don't
    // want the caller to use it incorrectly.  So we make things _always_
    // break when the caller forgets to call select() later.
    
    flush(0);
}


bool WvStream::pre_select(SelectInfo &si)
{
    time_t alarmleft = alarm_remaining();
    
    if (alarmleft == 0)
	return true; // alarm has rung

    if (!si.inherit_request)
	si.wants |= force;
    
    // handle read-ahead buffering
    if (si.wants.readable && inbuf.used() && inbuf.used() >= queue_min)
	return true; // already ready
    if (alarmleft >= 0
      && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft;
    return false;
}


bool WvStream::post_select(SelectInfo &si)
{
    // FIXME: need output buffer flush support for non FD-based streams
    // FIXME: need read_requires_writable and write_requires_readable
    //        support for non FD-based streams
    return false;
}


bool WvStream::_build_selectinfo(SelectInfo &si, time_t msec_timeout,
    bool readable, bool writable, bool isexcept, bool forceable)
{
    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
    
    if (forceable)
	si.wants = force;
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
	si.global_sure = s->pre_select(si);
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
#ifdef _WIN32
      && errno != WSAEINVAL // the sets might be empty
#endif
      )
    {
        seterr(errno);
    }
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
	si.global_sure = s->post_select(si) || si.global_sure;
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


void WvStream::force_select(bool readable, bool writable, bool isexception)
{
    force.readable |= readable;
    force.writable |= writable;
    force.isexception |= isexception;
}


void WvStream::undo_force_select(bool readable, bool writable, bool isexception)
{
    force.readable &= !readable;
    force.writable &= !writable;
    force.isexception &= !isexception;
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
    if (alarm_time.tv_sec && !running_callback)
    {
	WvTime now = wvtime();

	/* Time is going backward! */
	if (now < last_alarm_check)
	    alarm_time = tvdiff(alarm_time, tvdiff(last_alarm_check, now));

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
    assert(task);
    assert(taskman);
    assert(taskman->whoami() == task);
    
    if (msec_timeout >= 0)
	alarm(msec_timeout);
    
    running_callback = false;
    taskman->yield();
    alarm(-1);
    
    // FIXME: shouldn't we set running_callback = true here?
    
    // when we get here, someone has jumped back into our task.
    // We have to select(0) here because it's possible that the alarm was 
    // ticking _and_ data was available.  This is aggravated especially if
    // msec_delay was zero.  Note that running select() here isn't
    // inefficient, because if the alarm was expired then pre_select()
    // returned true anyway and short-circuited the previous select().
    TRACE("hello-%p\n", this);
    return !alarm_was_ticking || select(0);
}


void WvStream::terminate_continue_select()
{
    close();
    if (task)
    {
	while (task->isrunning())
	    taskman->run(*task);
	task->recycle();
	task = NULL;
    }
}


const WvAddr *WvStream::src() const
{
    return NULL;
}

