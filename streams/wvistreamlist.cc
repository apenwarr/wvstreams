/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvIStreamList holds a list of IWvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
#include "wvistreamlist.h"
#include "wvstringlist.h"
#include "wvstreamsdebugger.h"
#include "wvstrutils.h"

#include "wvassert.h"
#include "wvstrutils.h"

#ifndef _WIN32
#include "wvfork.h"
#endif

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#else
#define RUNNING_ON_VALGRIND false
#endif

// enable this to add some read/write trace messages (this can be VERY
// verbose)
#define STREAMTRACE 0
#if STREAMTRACE
# define TRACE(x, y...) fprintf(stderr, x, ## y)
#else
#ifndef _MSC_VER
# define TRACE(x, y...)
#else
# define TRACE
#endif
#endif

WvIStreamList WvIStreamList::globallist;


WvIStreamList::WvIStreamList():
    in_select(false), dead_stream(false)
{
    readcb = writecb = exceptcb = 0;
    auto_prune = true;
    if (this == &globallist)
    {
	globalstream = this;
#ifndef _WIN32
        add_wvfork_callback(WvIStreamList::onfork);
#endif
        set_wsname("globallist");
        add_debugger_commands();
    }
}


WvIStreamList::~WvIStreamList()
{
    close();
}


bool WvIStreamList::isok() const
{
    return WvStream::isok();
}


class BoolGuard
{
public:
    BoolGuard(bool &_guard_bool):
	guard_bool(_guard_bool)
    {
	assert(!guard_bool);
	guard_bool = true;
    }
    ~BoolGuard()
    {
	guard_bool = false;
    }
private:
    bool &guard_bool;
};


void WvIStreamList::pre_select(SelectInfo &si)
{
    //BoolGuard guard(in_select);
    bool already_sure = false;
    SelectRequest oldwant = si.wants;
    
    sure_thing.zap();
    
    time_t alarmleft = alarm_remaining();
    if (alarmleft == 0)
	already_sure = true;

    IWvStream *old_in_stream = WvCrashInfo::in_stream;
    const char *old_in_stream_id = WvCrashInfo::in_stream_id;
    WvCrashInfo::InStreamState old_in_stream_state = WvCrashInfo::in_stream_state;
    WvCrashInfo::in_stream_state = WvCrashInfo::PRE_SELECT;

    Iter i(*this);
    for (i.rewind(); i.next(); )
    {
	IWvStream &s(*i);
#if I_ENJOY_FORMATTING_STRINGS
	WvCrashWill will("doing pre_select for \"%s\" (%s)\n%s",
			 i.link->id, ptr2str(&s), wvcrash_read_will());
#else
	WvCrashInfo::in_stream = &s;
	WvCrashInfo::in_stream_id = i.link->id;
#endif
	si.wants = oldwant;
	s.pre_select(si);
	
	if (!s.isok())
	    already_sure = true;

	TRACE("after pre_select(%s): msec_timeout is %ld\n",
	      i.link->id, (long)si.msec_timeout);
    }

    WvCrashInfo::in_stream = old_in_stream;
    WvCrashInfo::in_stream_id = old_in_stream_id;
    WvCrashInfo::in_stream_state = old_in_stream_state;

    if (alarmleft >= 0 && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft;
    
    si.wants = oldwant;

    if (already_sure)
	si.msec_timeout = 0;
}


bool WvIStreamList::post_select(SelectInfo &si)
{
    //BoolGuard guard(in_select);
    bool already_sure = false;
    SelectRequest oldwant = si.wants;
    
    time_t alarmleft = alarm_remaining();
    if (alarmleft == 0)
	already_sure = true;

    IWvStream *old_in_stream = WvCrashInfo::in_stream;
    const char *old_in_stream_id = WvCrashInfo::in_stream_id;
    WvCrashInfo::InStreamState old_in_stream_state = WvCrashInfo::in_stream_state;
    WvCrashInfo::in_stream_state = WvCrashInfo::POST_SELECT;

    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	IWvStream &s(*i);
#if I_ENJOY_FORMATTING_STRINGS
	WvCrashWill will("doing post_select for \"%s\" (%s)\n%s",
			 i.link->id, ptr2str(&s), wvcrash_read_will());
#else
	WvCrashInfo::in_stream = &s;
	WvCrashInfo::in_stream_id = i.link->id;
#endif

	si.wants = oldwant;
	if (s.post_select(si))
	{
	    TRACE("post_select(%s) was true\n", i.link->id);
	    sure_thing.unlink(&s); // don't add it twice!
	    s.addRef();
	    sure_thing.append(&s, true, i.link->id);
	}
	else
	{
	    TRACE("post_select(%s) was false\n", i.link->id);
	    WvIStreamListBase::Iter j(sure_thing);
	    WvLink* link = j.find(&s);
	    
	    wvassert(!link, "stream \"%s\" (%s) was ready in "
		     "pre_select, but not in post_select",
		     link->id, ptr2str(link->data));
	}
	
	if (!s.isok())
	{
	    already_sure = true;
	    if (auto_prune)
		i.xunlink();
	}
    }
    
    WvCrashInfo::in_stream = old_in_stream;
    WvCrashInfo::in_stream_id = old_in_stream_id;
    WvCrashInfo::in_stream_state = old_in_stream_state;

    si.wants = oldwant;
    return already_sure || !sure_thing.isempty();
}


// distribute the callback() request to all children that select 'true'
void WvIStreamList::execute()
{
    static int level = 0;
    const char *id;
    level++;
    
    WvStream::execute();
    
    TRACE("\n%*sList@%p: (%d sure) ", level, "", this, sure_thing.count());
    
    IWvStream *old_in_stream = WvCrashInfo::in_stream;
    const char *old_in_stream_id = WvCrashInfo::in_stream_id;
    WvCrashInfo::InStreamState old_in_stream_state = WvCrashInfo::in_stream_state;
    WvCrashInfo::in_stream_state = WvCrashInfo::EXECUTE;

    WvIStreamListBase::Iter i(sure_thing);
    for (i.rewind(); i.next(); )
    {
#if STREAMTRACE
	WvIStreamListBase::Iter x(*this);
	if (!x.find(&i()))
	    TRACE("Yikes! %p in sure_thing, but not in main list!\n",
		  i.cur());
#endif
	IWvStream &s(*i);
	s.addRef();
	
	id = i.link->id;

	TRACE("[%p:%s]", &s, id);
	
	i.xunlink();
	
#if DEBUG
	if (!RUNNING_ON_VALGRIND)
	{
	    WvString strace_node("%s: %s", s.wstype(), s.wsname());
	    ::write(-1, strace_node, strace_node.len()); 
	}
#endif
#if I_ENJOY_FORMATTING_STRINGS
	WvCrashWill my_will("executing stream: %s\n%s",
			    id ? id : "unknown stream",
			    wvcrash_read_will());
#else
	WvCrashInfo::in_stream = &s;
	WvCrashInfo::in_stream_id = id;
#endif
	
	s.callback();
	s.release();
	
	// list might have changed!
	i.rewind();
    }
    
    WvCrashInfo::in_stream = old_in_stream;
    WvCrashInfo::in_stream_id = old_in_stream_id;
    WvCrashInfo::in_stream_state = old_in_stream_state;

    sure_thing.zap();

    level--;
    TRACE("[DONE %p]\n", this);
}

#ifndef _WIN32
void WvIStreamList::onfork(pid_t p)
{
    if (p == 0)
    {
        // this is a child process: don't inherit the global streamlist
        globallist.zap(false);
    }
}
#endif


void WvIStreamList::add_debugger_commands()
{
    WvStreamsDebugger::add_command("globallist", 0, debugger_globallist_run_cb, 0);
}


WvString WvIStreamList::debugger_globallist_run_cb(WvStringParm cmd,
    WvStringList &args,
    WvStreamsDebugger::ResultCallback result_cb, void *)
{
    debugger_streams_display_header(cmd, result_cb);
    WvIStreamList::Iter i(globallist);
    for (i.rewind(); i.next(); )
        debugger_streams_maybe_display_one_stream(static_cast<WvStream *>(i.ptr()),
                cmd, args, result_cb);
    
    return WvString::null;
}

