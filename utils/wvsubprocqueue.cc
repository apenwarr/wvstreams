/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A way to enqueue a series of WvSubProc objects.  See wvsubprocqueue.h.
 */
#include "wvsubprocqueue.h"
#include <unistd.h>


WvSubProcQueue::WvSubProcQueue(unsigned _maxrunning)
{
    maxrunning = _maxrunning;
}


WvSubProcQueue::~WvSubProcQueue()
{
}


void WvSubProcQueue::add(void *cookie, WvSubProc *proc)
{
    assert(proc);
    assert(!proc->running);
    if (cookie)
    {
	// search for other enqueued objects with this cookie
	EntList::Iter i(waitq);
	for (i.rewind(); i.next(); )
	{
	    if (i->cookie == cookie)
	    {
		// already enqueued; mark it as "redo" unless it's already
		// the last one.  That way we guarantee it'll still run
		// in the future from now, and it'll come later than anything
		// else in the queue, but it won't pointlessly run twice at
		// the end.
		Ent *e = i.ptr();
		if (i.next())
		    e->redo = true;
		delete proc;
		return;
	    }
	}
    }
    
    waitq.append(new Ent(cookie, proc), true);
}


void WvSubProcQueue::add(void *cookie,
			 const char *cmd, const char * const *argv)
{
    WvSubProc *p = new WvSubProc;
    p->preparev(cmd, argv);
    add(cookie, p);
}


bool WvSubProcQueue::cookie_running()
{
    EntList::Iter i(runq);
    for (i.rewind(); i.next(); )
	if (i->cookie)
	    return true;
    return false;
}


int WvSubProcQueue::go()
{
    int started = 0;
    
    //fprintf(stderr, "go: %d waiting, %d running\n",
    //	waitq.count(), runq.count());
    
    // first we need to clean up any finished processes
    {
	EntList::Iter i(runq);
	for (i.rewind(); i.next(); )
	{
	    Ent *e = i.ptr();
	    
	    e->proc->wait(0, true);
	    if (!e->proc->running)
	    {
		if (e->redo)
		{
		    // someone re-enqueued this task while it was
		    // waiting/running
		    e->redo = false;
		    i.xunlink(false);
		    waitq.append(e, true);
		}
		else
		    i.xunlink();
	    }
	}
    }
    
    while (!waitq.isempty() && runq.count() < maxrunning)
    {
	EntList::Iter i(waitq);
	for (i.rewind(); i.next(); )
	{
	    // elements with cookies are "sync points" in the queue;
	    // they guarantee that everything before that point has
	    // finished running before they run, and don't let anything
	    // after them run until they've finished.
	    if (i->cookie && !runq.isempty())
		goto out;
	    if (cookie_running())
		goto out;
	    
	    // jump it into the running queue, but be careful not to
	    // delete the object when removing!
	    Ent *e = i.ptr();
	    i.xunlink(false);
	    runq.append(e, true);
	    e->proc->start_again();
	    started++;
	    break;
	}
    }
    
out:
    assert(runq.count() <= maxrunning);
    return started;
}


unsigned WvSubProcQueue::running() const
{
    return runq.count();
}


unsigned WvSubProcQueue::remaining() const
{
    return runq.count() + waitq.count();
}


bool WvSubProcQueue::isempty() const
{
    return runq.isempty() && waitq.isempty();
}


void WvSubProcQueue::finish()
{
    while (!isempty())
    {
	go();
	if (!isempty())
	    usleep(100*1000);
    }
}
