/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A more convenient way to use WvSubProcQueue.  See wvsubprocqueuestream.h.
 */
#include "wvsubprocqueuestream.h"


WvSubProcQueueStream::WvSubProcQueueStream(int _maxrunning)
    : WvSubProcQueue(_maxrunning), log("Subproc Queue", WvLog::Debug5)
{
    alarm(0);
}


WvSubProcQueueStream::~WvSubProcQueueStream()
{
}


void WvSubProcQueueStream::execute()
{
    int started = WvSubProcQueue::go();
    log("Started %s processes (%s running, %s waiting)\n",
	started, running(), remaining() - running());
    if (!remaining())
	alarm(1000); // nothing is even in the queue; come back later.
    else if (started)
	alarm(0); // we're busy; go fast if possible
    else
	alarm(100); // no processes were ready *this* time; wait longer
}


