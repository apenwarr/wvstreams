/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * wvfork() just runs fork(), but it closes all file descriptors that
 * are flagged close-on-exec, since we don't necessarily always run
 * exec() after we fork()...
 *
 * This fixes the year-old mystery bug where WvTapeBackup caused
 * watchdog reboots because the CHILD process wasn't touching it, and
 * it was already open before the fork()...
 *
 * If you want to explicitly leave a file descriptor OPEN, even if
 * it's marked close-on-exec, then add the fd number to dontclose, and
 * pass that to wvfork().  This is mainly useful for WvLoopbacks --
 * you may want certain ones open or closed depending on which call to
 * wvfork() you're making.  (for WvTapeBackup, you want the three
 * backup loopbacks open, and, say, any WvResolver loopbacks closed.)
 */

#include <fcntl.h>

#include "wvfork.h"
#include "wvlinklist.h"

#define MAX_FD sysconf(_SC_OPEN_MAX) + 1

DeclareWvList(WvForkCallback);
static WvForkCallbackList *callbacks;


// note: this shouldn't really be needed (it would be better to use a simple
// static list), but might be needed if your dynamic linker (ld.so) runs
// global constructors in the wrong order.
static WvForkCallbackList &get_callbacks()
{
    if (!callbacks)
	callbacks = new WvForkCallbackList;
    return *callbacks;		
}


void add_wvfork_callback(WvForkCallback cb)
{
#if 0
    // be sure we don't add this twice
    WvForkCallbackList::Iter i(get_callbacks());
    for (i.rewind(); i.next(); )
        if (*i == cb) return;
#endif
    get_callbacks().append(new WvForkCallback(cb), true);
}

#if 0
void remove_wvfork_callback(WvForkCallback cb)
{
    WvForkCallbackList::Iter i(get_callbacks());
    for (i.rewind(); i.next(); )
        if (*i == cb) i.xunlink();
}
#endif

pid_t wvfork(int dontclose1, int dontclose2)
{
    intTable t(1);
    if (dontclose1 >= 0)
	t.add(&dontclose1, false);
    if (dontclose2 >= 0)
	t.add(&dontclose2, false);
    return (wvfork(t));
}

pid_t wvfork_start(int *waitfd)
{
    int waitpipe[2];

    if (pipe(waitpipe) < 0)
	return -1;

    pid_t pid = fork();

    WvForkCallbackList::Iter i(get_callbacks());
    for (i.rewind(); i.next(); )
    {
        WvForkCallback *cb = i.ptr();
        (*cb)(pid);
    }

    if (pid < 0)
	return pid;
    else if (pid > 0)
    {
	// parent process. close its writing end of the pipe and wait
	// for its reading end to close.
	char buf;
	close(waitpipe[1]);
	read(waitpipe[0], &buf, 1);
	close(waitpipe[0]);
    }
    else
    {
	// child process. close its reading end of the pipe.
	close(waitpipe[0]);
	*waitfd = waitpipe[1];
    }

    return pid;
}

pid_t wvfork(intTable &dontclose)
{
    int waitfd = -1;
    pid_t pid = wvfork_start(&waitfd);

    if (pid != 0)
    {
	// parent or error
	return pid;
    }

    // child process
    // check the close-on-exec flag of all file descriptors
    for (int fd = 0; fd < MAX_FD; fd++)
    {
	if (!dontclose[fd] && fd != waitfd &&
	    (fcntl(fd, F_GETFD) & FD_CLOEXEC) > 0)
	    close(fd);
    }

    close(waitfd);

    return pid;
}
