/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * A class for reliably starting/stopping subprocesses.  See wvsubproc.h.
 */
#include "wvsubproc.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>


WvSubProc::WvSubProc()
{
    pid = -1;
    running = false;
    estatus = 0;
}


WvSubProc::~WvSubProc()
{
    // we need to kill the process here, or else we could leave
    // Zombies lying around...
    stop(100);
}


int WvSubProc::start(const char cmd[], ...)
{
    va_list ap;
    int nargs, count, retval;
    char *cptr;
    char **argv;
    
    assert(!running);
    
    // count the number of arguments
    va_start(ap, cmd);
    for (nargs = 0; (cptr = va_arg(ap, char *)) != NULL; nargs++)
	;
    va_end(ap);
    
    // allocate enough space for all the args, plus NULL.
    argv = new char* [nargs+1];
    
    // copy the arguments into the array
    va_start(ap, cmd);
    for (count = 0; count < nargs; count++)
	argv[count] = va_arg(ap, char *);
    argv[nargs] = NULL;
    
    retval = startv(cmd, argv);
    
    delete[] argv;
    
    return retval;
}


int WvSubProc::startv(const char cmd[], char * const *argv)
{
    running = false;
    estatus = 0;
    
    pid = fork();
    //fprintf(stderr, "pid for '%s' is %d\n", cmd, pid);
    
    if (!pid)
    {
	// child process
	 
	// set the process group of this process, so "negative" kill will kill
	// everything in the whole session, not just the main process.
	setpgid(0,0);
	
	// set up any extra environment variables
	WvStringList::Iter i(env);
	for (i.rewind(); i.next(); )
	    putenv(i().edit());
	
	// run the subprocess.
	execvp(cmd, argv);
	
	// if we get this far, just make sure we exit, not return.  The code 242
	// should be somewhat recognizable by the calling process so we know
	// something is up.
	_exit(242);
    }
    else if (pid > 0)
    {
	// parent process
	running = true;
    }
    else if (pid < 0)
	return -errno;
    
    return 0; // ok
}


void WvSubProc::kill(int sig)
{
    assert(!running || pid > 1);
    
    if (running)
    {
	// if the process group has disappeared, kill the main process instead
	if (::kill(-pid, sig) < 0 && errno == ESRCH)
	    kill_primary(sig);
    }
}


void WvSubProc::kill_primary(int sig)
{
    assert(!running || pid > 1);
    
    if (running)
	::kill(pid, sig);
}


void WvSubProc::stop(time_t msec_delay)
{
    if (!running) return;
    
    wait(0);
    
    if (running)
    {
	kill(SIGTERM);
	wait(msec_delay);
    }
    
    if (running)
    {
	kill(SIGKILL);
	wait(-1);
    }
}


static long msecdiff(struct timeval &a, struct timeval &b)
{
    long secdiff, usecdiff;
    
    secdiff = a.tv_sec - b.tv_sec;
    usecdiff = a.tv_usec - b.tv_usec;
    
    return secdiff*1000 + usecdiff/1000;
}


void WvSubProc::wait(time_t msec_delay)
{
    int status;
    pid_t dead_pid;
    struct timeval tv1, tv2;
    struct timezone tz;
    
    assert(!running || pid > 1);
    
    if (!running) return;
    
    gettimeofday(&tv1, &tz);
    tv2 = tv1;
    
    do
    {
	// note: there's a small chance that the child process
	// hasn't run setpgrp() yet, so no processes will be available
	// for "-pid".  Wait on pid if it fails.
	// 
	// also note: waiting on a process group is actually useless since you
	// can only get notifications for your direct descendants.  We have
	// to "kill" with a zero signal instead to try to detect whether they've
	// died or not.
	dead_pid = waitpid(-pid, &status, 
			   (msec_delay >= 0) ? WNOHANG : 0);
	if (dead_pid < 0 && errno == ECHILD)
	    dead_pid = waitpid(pid, &status, 
			       (msec_delay >= 0) ? WNOHANG : 0);
	
	//fprintf(stderr, "%ld: dead_pid=%d; pid=%d\n",
	//	msecdiff(tv2, tv1), dead_pid, pid);
	if (dead_pid < 0)
	{
	    // all relevant children are dead!
	    if (errno == ECHILD)
	    {
		if (::kill(-pid, 0) && errno == ESRCH)
		{
		    running = false;
		    pid = -1;
		}
	    }
	    else
		perror("WvSubProc::wait");
	}
	else if (dead_pid == pid)
	{
	    // it's the main process - save its status.
	    estatus = status;
	}
	
	if (running && msec_delay != 0)
	{
	    // wait a while, so we're not spinning _too_ fast in a loop
	    usleep(50*1000);
	}
	
	gettimeofday(&tv2, &tz);
	
    } while (running && msec_delay 
	     && (msec_delay < 0 || msecdiff(tv2, tv1) < msec_delay));
}
