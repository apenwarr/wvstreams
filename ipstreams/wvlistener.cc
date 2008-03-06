/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A base implementation for "listeners", streams that spawn other streams
 * from (presumably) incoming connections.
 */ 
#include "wvlistener.h"
#include "wvistreamlist.h"
#include "wvaddr.h"
#include "wvmoniker.h"

UUID_MAP_BEGIN(WvListener)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IWvStream)
  UUID_MAP_ENTRY(IWvListener)
  UUID_MAP_END


IWvListener *IWvListener::create(WvString moniker, IObject *obj)
{
    IWvListener *l = wvcreate<IWvListener>(moniker, obj);
    if (!l)
    {
	l = new WvNullListener();
	l->seterr_both(EINVAL, "Unknown moniker '%s'", moniker);
    }
    return l;
}


WvListener::WvListener(IWvStream *_cloned)
{
    cloned = _cloned;
    wrapper = 0;
}
    

WvListener::~WvListener()
{
    if (cloned)
	WVRELEASE(cloned);
    WvIStreamList::globallist.unlink(this);
}


static IWvStream *wrapper_runner(IWvListenerWrapper wrapper,
				 IWvStream *s)
{
    return wrapper(s);
}


void WvListener::addwrap(IWvListenerWrapper _wrapper)
{
    // What the heck is this, you ask?
    // The idea is that we can support multiple layers of wrappers by
    // creating recursive callbacks.  When we add a wrapper and one already
    // exists, we want to create a new one to essentially do newer(older(s))
    // ...but only when it finally gets called.
    if (wrapper)
	wrapper = wv::bind(&wrapper_runner, _wrapper, _1);
    else
	wrapper = _wrapper;
}


void WvListener::callback()
{  
    if (acceptor)
    {
	IWvStream *s = accept();
	if (s) acceptor(s);
    }
}


IWvStream *WvListener::wrap(IWvStream *s)
{
    if (wrapper && s)
	return wrapper(s);
    else
	return s;
}
    

IWvListenerCallback WvListener::onaccept(IWvListenerCallback _cb)
{
    IWvListenerCallback old = acceptor;
    acceptor = _cb;
    return old;
}


void WvListener::runonce(time_t msec_delay)
{
    callback();
}


static WvStringAddr nulladdr("ERROR", WvEncap::Unknown);
const WvAddr *WvNullListener::src() const
{
    return &nulladdr;
}


