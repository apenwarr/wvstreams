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
#include <time.h>
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

WvTaskMan *WvStream::taskman;

static void normalize(struct timeval &tv)
{
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec %= 1000000;
}


WvStream::WvStream(int _fd) : callfunc(NULL)
{
    init();
    rwfd = _fd;
}


void WvStream::init()
{
    wvstream_execute_called = false;
    userdata = NULL;
    errnum = 0;
    max_outbuf_size = 0;
    outbuf_delayed_flush = false;
    is_auto_flush = true;
    alarm_was_ticking = false;
    force.readable = true;
    force.writable = force.isexception = false;
    read_requires_writable = write_requires_readable = NULL;
    running_callback = false;
    queue_min = 0;
    autoclose_time = 0;
    alarm_time.tv_sec = alarm_time.tv_usec = 0;
    
    // magic multitasking support
    uses_continue_select = false;
    personal_stack_size = 65536;
    task = NULL;
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
}


void WvStream::close()
{
    int rfd = getrfd(), wfd = getwfd();
    
    flush(2000); // fixme: should not hardcode this stuff
    if (rfd >= 0)
	::close(getrfd());
    if (wfd >= 0 && wfd != rfd)
	::close(getwfd());
    rwfd = -1;
}


void WvStream::autoforward(WvStream &s)
{
    setcallback(autoforward_callback, &s);
    read_requires_writable = &s;
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
    if (s->callfunc)
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
	alarm_time.tv_sec = alarm_time.tv_usec = 0;
	alarm_was_ticking = true;
    }
    else
	alarm_was_ticking = false;
    
    assert(!uses_continue_select || personal_stack_size >= 1024);
    
//    if (1)
    if (uses_continue_select && personal_stack_size >= 1024)
    {
	if (!taskman)
	    taskman = new WvTaskMan;
    
	if (!task)
	{
	    TRACE("(!)");
	    task = taskman->start("streamexec", _callback, this,
				  personal_stack_size);
	}
	else if (!task->isrunning())
	{
	    TRACE("(.)");
	    fflush(stderr);
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


// by default, we use the same fd for reading and writing
int WvStream::getrfd() const
{
    return rwfd;
}


// by default, we use the same fd for reading and writing
int WvStream::getwfd() const
{
    return rwfd;
}


int WvStream::getfd() const
{
    int rfd = getrfd(), wfd = getwfd();
    assert(rfd == wfd);
    return rfd;
}


bool WvStream::isok() const
{
    return (getrfd() != -1) && (getwfd() != -1) && WvError::isok();
}


void WvStream::seterr(int _errnum)
{
    WvError::seterr(_errnum);
    close();
}


size_t WvStream::read(WvBuffer &outbuf, size_t count)
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


size_t WvStream::write(WvBuffer &inbuf, size_t count)
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


size_t WvStream::uread(void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int in = ::read(getrfd(), buf, count);
    
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
    if (! outbuf_delayed_flush && ! outbuf.used())
    {
	wrote = uwrite(buf, count);
        count -= wrote;
        (const unsigned char*)buf += wrote;
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
    if (! outbuf_delayed_flush)
    {
        if (is_auto_flush)
            flush(0);
        else
            flush_outbuf(0);
    }
    return wrote;
}


size_t WvStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    int out = ::write(getwfd(), buf, count);
    
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


// NOTE:  wait_msec is implemented wrong, but it has cleaner code this way
// and can at least handle wait vs wait forever vs wait never.
char *WvStream::getline(time_t wait_msec, char separator,
    int readahead)
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
            // the following cast is of dubious quality...
	    buf = const_cast<unsigned char*>(inbuf.get(i));
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

	// make select not return true until more data is available
	if (inbuf.used())
	    queuemin(inbuf.used() + 1);
	
	// note: this _always_ does the select, even if wait_msec < 0.
	// That's good, because the fd might be nonblocking!
	if (uses_continue_select)
	{
	    if (!continue_select(wait_msec) && isok() && wait_msec >= 0)
		return NULL;
	}
	else
	{
	    if (!select(wait_msec) && isok() && wait_msec >= 0)
		return NULL;
	}
	
	if (!isok())
	    return NULL;

	// read a few bytes
	buf = inbuf.alloc(readahead);
	i = uread(buf, readahead);
	inbuf.unalloc(readahead - i);
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
    // flush any other internal buffers a stream might have
    flush_internal(msec_timeout);

    // flush outbuf
    flush_outbuf(msec_timeout);
}


void WvStream::flush_outbuf(time_t msec_timeout)
{
    // flush outbuf
    while (isok() && outbuf.used())
    {
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

    if (isok())
    {
        // handle autoclose
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
}


void WvStream::flush_internal(time_t msec_timeout)
{
    // once outbuf emptied, that's it for most streams
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
    int rfd, wfd;
    
    time_t alarmleft = alarm_remaining();
    
    if (alarmleft == 0)
	return true; // alarm has rung
    
    // handle read-ahead buffering
    if (si.wants.readable && inbuf.used() && inbuf.used() >= queue_min)
	return true; // already ready
    
    rfd = getrfd();
    wfd = getwfd();
    
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
    
    if (alarmleft >= 0
      && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft;
    
    return false;
}


bool WvStream::post_select(SelectInfo &si)
{
    size_t outbuf_used = outbuf.used();
    int rfd = getrfd(), wfd = getwfd();
    bool val;
    
    // flush the output buffer if possible
    if (wfd >= 0 
	&& (outbuf_used || autoclose_time)
	&& FD_ISSET(wfd, &si.write))
    {
        flush_outbuf(0);
	
	// flush_outbuf() might have closed the file!
	if (!isok()) return false;
    }
    
    val = ((rfd >= 0 && FD_ISSET(rfd, &si.read)) ||
	    (wfd >= 0 && FD_ISSET(wfd, &si.write)) ||
	    (rfd >= 0 && FD_ISSET(rfd, &si.except)) ||
	    (wfd >= 0 && FD_ISSET(wfd, &si.except)));
    
    if (val && si.wants.readable && read_requires_writable
      && !read_requires_writable->select(0, false, true))
	return false;
    if (val && si.wants.writable && write_requires_readable
      && !write_requires_readable->select(0, true, false))
	return false;
    
    return val;
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

    if (!isok()) return false;

    bool sure = pre_select(si);
    if (sure)
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

    // handle errors
    if (sel < 0 &&
        errno != EAGAIN && errno != EINTR && errno != ENOBUFS)
        seterr(errno);
    return sel;
}


bool WvStream::_process_selectinfo(SelectInfo &si)
{
    if (!isok()) return false;
    return post_select(si);
}


bool WvStream::_select(time_t msec_timeout,
    bool readable, bool writable, bool isexcept, bool forceable)
{
    SelectInfo si;
    bool sure = _build_selectinfo(si, msec_timeout,
        readable, writable, isexcept, forceable);

    int sel = _do_select(si);
    if (sel > 0)
        sure = _process_selectinfo(si) || sure; // note the order
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
    struct timezone tz;
    
    if (msec_timeout >= 0)
    {
	gettimeofday(&alarm_time, &tz);
	alarm_time.tv_sec += msec_timeout / 1000;
	alarm_time.tv_usec += (msec_timeout % 1000) * 1000;
	normalize(alarm_time);
    }
    else
    {
	// cancel alarm
	alarm_time.tv_sec = alarm_time.tv_usec = 0;
    }
}


time_t WvStream::alarm_remaining()
{
    struct timeval &a = alarm_time;
    
    if (a.tv_sec && !running_callback)
    {
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday(&tv, &tz);
	normalize(tv);
	
	if (a.tv_sec < tv.tv_sec
	    || (   a.tv_sec  == tv.tv_sec 
		&& a.tv_usec <= tv.tv_usec))
	{
	    return 0;
	}
	else if (a.tv_sec > tv.tv_sec)
	{
	    return ((a.tv_sec - tv.tv_sec) * 1000
		    + (a.tv_usec - tv.tv_usec) / 1000);
	}
	else // a.tv_sec == tv.tv_sec
	{
	    return (a.tv_usec - tv.tv_usec) / 1000;
	}
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
    
    // when we get here, someone has jumped back into our task.
    // We have to select(0) here because it's possible that the alarm was 
    // ticking _and_ data was available.  This is aggravated especially if
    // msec_delay was zero.  Note that running select() here isn't
    // inefficient, because if the alarm was expired then pre_select()
    // returned true anyway and short-circuited the previous select().
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
