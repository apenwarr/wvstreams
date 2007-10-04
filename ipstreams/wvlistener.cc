/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A base implementation for "listeners", streams that spawn other streams
 * from (presumably) incoming connections.
 */ 
#include "wvlistener.h"
#include "wvistreamlist.h"

UUID_MAP_BEGIN(WvListener)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IWvStream)
  UUID_MAP_ENTRY(IWvListener)
  UUID_MAP_END


WvListener::WvListener(IWvStream *_cloned)
{
    cloned = _cloned;
}
    

WvListener::~WvListener()
{
    if (cloned)
	WVRELEASE(cloned);
    WvIStreamList::globallist.unlink(this);
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
