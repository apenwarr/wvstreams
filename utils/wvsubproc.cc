/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A class for reliably starting/stopping subprocesses.  See
 * wvsubproc.h.
 */
#include "wvsubproc.h"
#include "wvtimeutils.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include "wvfork.h"

WvSubProc::WvSubProc()
{
    pid = -1;
    running = false;
    estatus = 0;
}


WvSubProc::~WvSubProc()
{
    // we need to kill the process here, or else we could leave
    // zombies lying around...
    stop(100);
}


int WvSubProc::_startv(const char cmd[], const char * const *argv)
{
    int waitfd = -1;
    
    pid = fork(&waitfd);
    //fprintf(stderr, "pid for '%s' is %d\n", cmd, pid);
    
    if (!pid) // child process
    {
	// unblock the parent.
	close(waitfd);
	
	// run the subprocess.
	execvp(cmd, (char * const *)argv);
	
	// if we get this far, just make sure we exit, not return.
	// The code 242 should be somewhat recognizable by the calling
	// process so we know something is up.
	_exit(242);
    }
    else if (pid > 0) // parent process
	running = true;
    else if (pid < 0)
	return pid;
    
    return 0; // ok
}


void WvSubProc::prepare(const char cmd[], ...)
{
    va_list ap;
    va_start(ap, cmd);
    preparev(cmd, ap);
    va_end(ap);
}


void WvSubProc::preparev(const char cmd[], va_list ap)
{
    const char *argptr;
    
    // remember the command so start_again() will work
    last_cmd = cmd;
    last_args.zap();
    while ((argptr = va_arg(ap, const char *)) != NULL)
	last_args.append(new WvString(argptr), true);
}


void WvSubProc::preparev(const char cmd[], const char * const *argv)
{
    const char * const *argptr;
    
    // remember the command so start_again() will work
    last_cmd = cmd;
    last_args.zap();
    for (argptr = argv; argptr && *argptr; argptr++)
	last_args.append(new WvString(*argptr), true);
}


int WvSubProc::start(const char cmd[], ...)
{
    va_list ap;
    va_start(ap, cmd);
    preparev(cmd, ap);
    va_end(ap);
    
    return start_again();
}


int WvSubProc::startv(const char cmd[], const char * const *argv)
{
    preparev(cmd, argv);
    return start_again();
}


int WvSubProc::start_again()
{
    int retval;
    const char **argptr;
    
    assert(!!last_cmd);
    
    // create a new argv array from our stored values
    const char **argv = new const char*[last_args.count() + 1];
    WvStringList::Iter i(last_args);
    for (argptr = argv, i.rewind(); i.next(); argptr++)
	*argptr = *i;
    *argptr = NULL;
    
    // run the program
    retval = _startv(last_cmd, argv);
    
    // clean up
    delete[] argv;
    
    return retval;
}


int WvSubProc::fork(int *waitfd)
{
    running = false;
    estatus = 0;

    pid = wvfork_start(waitfd);

    if (!pid)
    {
	// child process
	 
	// set the process group of this process, so "negative" kill
	// will kill everything in the whole session, not just the
	// main process.
	setpgid(0,0);

	// set up any extra environment variables
	WvStringList::Iter i(env);
	for (i.rewind(); i.next(); )
	    putenv(i().edit());
    }
    else if (pid > 0)
    {
	// parent process
	running = true;
    }
    else if (pid < 0)
	return -errno;
    
    return pid;
}


void WvSubProc::kill(int sig)
{
    assert(!running || pid > 0 || !old_pids.isempty());
    
    if (pid > 0)
    {
	// if the process group has disappeared, kill the main process
	// instead.
	if (::kill(-pid, sig) < 0 && errno == ESRCH)
	    kill_primary(sig);
    }
    
    // kill leftover subprocesses too.
    pid_tList::Iter i(old_pids);
    for (i.rewind(); i.next(); )
    {
	pid_t subpid = *i;
	if (::kill(-subpid, sig) < 0 && errno == ESRCH)
	    ::kill(subpid, sig);
    }
}


void WvSubProc::kill_primary(int sig)
{
    assert(!running || pid > 0 || !old_pids.isempty());
    
    if (running && pid > 0)
	::kill(pid, sig);
}


void WvSubProc::stop(time_t msec_delay, bool kill_children)
{
    wait(0);
    
    if (running)
    {
	if (kill_children)
	    kill(SIGTERM);
	else
	    kill_primary(SIGTERM);

	wait(msec_delay, kill_children);
    }
    
    if (running)
    {
	if (kill_children)
	    kill(SIGKILL);
	else
	    kill_primary(SIGKILL);

	wait(-1, kill_children);
    }
}


void WvSubProc::wait(time_t msec_delay, bool wait_children)
{
    bool xrunning;
    int status;
    pid_t dead_pid;
    struct timeval tv1, tv2;
    struct timezone tz;
    
    assert(!running || pid > 0 || !old_pids.isempty());

    // running might be false if the parent process is dead and you called
    // wait(x, false) before.  However, if we're now doing wait(x, true),
    // we want to keep going until the children are dead too.
    xrunning = (running || (wait_children && !old_pids.isempty()));
    
    if (!xrunning) return;
    
    gettimeofday(&tv1, &tz);
    tv2 = tv1;
    
    do
    {
	if (pid > 0)
	{
	    // waiting on a process group is unfortunately useless
	    // since you can only get notifications for your direct
	    // descendants.  We have to "kill" with a zero signal instead
	    // to try to detect whether they've died or not.
	    dead_pid = waitpid(pid, &status, (msec_delay >= 0) ? WNOHANG : 0);
	
	    //fprintf(stderr, "%ld: dead_pid=%d; pid=%d\n",
	    //	msecdiff(tv2, tv1), dead_pid, pid);
	    
	    if (dead_pid == pid 
		|| (dead_pid < 0 && (errno == ECHILD || errno == ESRCH)))
	    {
		// the main process is dead - save its status.
		estatus = status;
		old_pids.append(new pid_t(pid), true);
		pid = -1;
	    }
	    else if (dead_pid < 0)
		perror("WvSubProc::waitpid");
	}
	
	// no need to do this next part if the primary subproc isn't dead yet
	if (pid < 0)
	{
	    pid_tList::Iter i(old_pids);
	    for (i.rewind(); i.next(); )
	    {
		pid_t subpid = *i;
		if (::kill(-subpid, 0) && errno == ESRCH)
		    i.xunlink();
	    }
	    
	    // if the primary is dead _and_ we either don't care about
	    // children or all our children are dead, then the subproc
	    // isn't actually running.
	    if (!wait_children || old_pids.isempty())
		xrunning = false;
	}

	// wait a while, so we're not spinning _too_ fast in a loop
	if (xrunning && msec_delay != 0)
	    usleep(50*1000);
	
	gettimeofday(&tv2, &tz);
	
    } while (xrunning && msec_delay
	     && (msec_delay < 0 || msecdiff(tv2, tv1) < msec_delay));

    if (!xrunning)
	running = false;
}
