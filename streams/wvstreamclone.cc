/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * WvStreamClone simply forwards all requests to the "cloned" stream.
 * 
 * NOTE: this file is a pain to maintain, because many of these functions
 * are almost (but not quite) exactly like the ones in WvStream.  If
 * WvStream changes, you need to change this too.
 * 
 * See wvstreamclone.h.
 */
#include "wvstreamclone.h"
#include <errno.h>
#include <time.h>


WvStreamClone::~WvStreamClone()
{
    // do NOT close the cloned stream!
}


void WvStreamClone::close()
{
    if (s())
	s()->close();
}


int WvStreamClone::getfd() const
{
    if (s())
	return s()->getfd();
    return -1;
}


size_t WvStreamClone::uread(void *buf, size_t size)
{
    if (s())
	return s()->read(buf, size);
    else
	return 0;
}


size_t WvStreamClone::uwrite(const void *buf, size_t size)
{
    // we use s()->uwrite() here, not write(), since we want the _clone_
    // to own the output buffer, not the main stream.
    if (s())
	return s()->uwrite(buf, size);
    else
	return 0;
}


bool WvStreamClone::isok() const
{
    if (errnum)
	return false;
    if (s())
	return s()->isok();
    return false;
}


int WvStreamClone::geterr() const
{
    if (errnum)
	return errnum;
    if (s())
	return s()->geterr();
    return EIO;
}


const char *WvStreamClone::errstr() const
{
    if (errnum)
	return WvStream::errstr();
    if (s())
	return s()->errstr();
    return "No child stream!";
}


bool WvStreamClone::select_setup(SelectInfo &si)
{
    bool oldrd, oldwr, oldex, result;
    time_t alarmleft = alarm_remaining();
    
    if (alarmleft == 0 && !select_ignores_buffer)
	return true; // alarm has rung
    
    if (si.readable && !select_ignores_buffer && inbuf.used() 
	   && inbuf.used() >= queue_min)
	return true;   // sure_thing if anything in WvStream buffer
    
    if (alarmleft >= 0
      && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft;
    
    if (s() && s()->isok())
    {
	oldrd = si.readable;
	oldwr = si.writable;
	oldex = si.isexception;
	
	if (force.readable)
	    si.readable = true;
	if (outbuf.used() || autoclose_time || force.writable)
	    si.writable = true;
	if (force.isexception)
	    si.isexception = true;
	
	result = s()->select_setup(si);
	
	si.readable = oldrd;
	si.writable = oldwr;
	si.isexception = oldex;
	return result;
    }
    
    return false;
}


bool WvStreamClone::test_set(SelectInfo &si)
{
    bool oldrd, oldwr, oldex, val;
    
    if (s() && (outbuf.used() || autoclose_time))
	flush(0);

    if (s() && s()->isok())
    {
	oldrd = si.readable;
	oldwr = si.writable;
	oldex = si.isexception;
	
	val = s()->test_set(si);
	
	si.readable = oldrd;
	si.writable = oldwr;
	si.isexception = oldex;
	
	return val;
    }
    return false;
}


const WvAddr *WvStreamClone::src() const
{
    if (s())
	return s()->src();
    return NULL;
}


void WvStreamClone::execute()
{
    if (s()) s()->callback();
}
