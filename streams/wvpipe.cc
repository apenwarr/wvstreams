/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Implementation of a WvPipe stream.  WvPipes allow you to create a new
 * process, attaching its stdin/stdout to a WvStream.
 * 
 * See wvpipe.h for more information.
 */
#include "wvpipe.h"
#include "wvsplitstream.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <assert.h>


WvPipe::WvPipe(const char *program, char **argv,
	       bool writable, bool readable, bool catch_stderr,
	       int stdin_fd, int stdout_fd, int stderr_fd)
{
    setup(program, argv, writable, readable, catch_stderr,
	  stdin_fd, stdout_fd, stderr_fd);
}


WvPipe::WvPipe(const char *program, char **argv,
	       bool writable, bool readable, bool catch_stderr,
	       WvStream *stdin_str, WvStream *stdout_str,
	       WvStream *stderr_str)
{
    int fd0 = 0, fd1 = 1, fd2 = 2;
    if (stdin_str)
	fd0 = stdin_str->getfd();
    if (stdout_str)
	fd1 = stdout_str->getfd();
    if (stderr_str)
	fd2 = stderr_str->getfd();
    setup(program, argv, writable, readable, catch_stderr, fd0, fd1, fd2);
}


WvPipe::WvPipe(const char *program, char **argv,
	       bool writable, bool readable, bool catch_stderr,
	       WvSplitStream *stdio_str)
{
    if (stdio_str)
    {
	int rfd = stdio_str->getrfd(), wfd = stdio_str->getwfd();
	setup(program, argv, writable, readable, catch_stderr,
	      rfd, wfd, wfd);
    }
    else
	setup(program, argv, writable, readable, catch_stderr, 0, 1, 2);
}


void WvPipe::setup(const char *program, char **argv,
	      bool writable, bool readable, bool catch_stderr,
	      int stdin_fd, int stdout_fd, int stderr_fd)
{
    int socks[2];
    
    pid = 0;
    estatus = -1;

    if (!program || !argv)
    {
	errnum = EINVAL;
	return;
    }
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
    {
	errnum = errno;
	return;
    }
    
    pid = fork();
    if (pid < 0)
    {
	pid = 0;
	errnum = errno;
	::close(socks[0]);
	::close(socks[1]);
	return;
    }
    
    if (!pid)   // child process
    {
	::close(socks[0]);
	
	if (writable)
	    dup2(socks[1], 0); // writable means redirect child stdin
	else if (stdin_fd == -1)
	    ::close(0);
	else
	    dup2(stdin_fd, 0);
	if (readable)
	    dup2(socks[1], 1); // readable means we redirect child stdout
	else if (stdout_fd == -1)
	    ::close(1);
	else
	    dup2(stdout_fd, 1);
	if (catch_stderr)
	    dup2(socks[1], 2); // but catch_stderr does what you think
	else if (stderr_fd == -1)
	    ::close(2);
	else
	    dup2(stderr_fd, 2);
	
	fcntl(0, F_SETFD, 0);  // never close stdin
	fcntl(1, F_SETFD, 0);  // never close stdout
	fcntl(2, F_SETFD, 0);  // never close stderr
	
	// this will often fail, but when it does work it is probably
	// the Right Thing To Do (tm)
	if (!readable && stdout_fd != 1)
	{
	    setsid();
	    ioctl(1, TIOCSCTTY, 1);
	}
	
	// now run the program.  If it fails, use _exit() so no destructors
	// get called and make a mess.
	if (execvp(program, argv))
	    _exit(242);
    }

    // otherwise, parent process
    fd = socks[0];
    ::close(socks[1]);
}


// send the child process a signal
void WvPipe::kill(int signum)
{
    if (pid)
	::kill(pid, signum);
}


// wait for the child to die
int WvPipe::finish()
{
    while (!child_exited())
	usleep(100*1000);
    
    return exit_status();
}


// determine if the child process has exited.
// Note:  if the child forks off, this does not necessarily mean that
//    the stream is invalid!  (use isok() for that as usual)
bool WvPipe::child_exited()
{
    int status;
    pid_t dead_pid;
    
    if (!pid) return true;
    
    dead_pid = waitpid(pid, &status, WNOHANG);
    if (dead_pid != pid)
	return false;
    estatus = status;
    pid = 0;
    return true;
}


// if child_exited(), return true if it died because of a signal, or
// false if it died due to a call to exit().
bool WvPipe::child_killed() const
{
    int st = estatus;
    assert (WIFEXITED(st) || WIFSIGNALED(st));
    return WIFSIGNALED(st);
}


// return the numeric exit status of the child (if it exited) or the
// signal that killed the child (if it was killed).
int WvPipe::exit_status() const
{
    int st = estatus;
    assert (WIFEXITED(st) || WIFSIGNALED(st));
    if (child_killed())
	return WTERMSIG(st);
    else
	return WEXITSTATUS(st);
}


// make sure our subtask ends up dead!
WvPipe::~WvPipe()
{
    int status, count;
    pid_t dead_pid;
    
    close();
    
    if (!pid) return;

    dead_pid = waitpid(pid, &status, WNOHANG);
    if (dead_pid == 0)
    {
	kill(SIGTERM);
	
	for (count = 20; count > 0; count--)
	{
	    dead_pid = waitpid(pid, &status, WNOHANG);
	    if (dead_pid == pid)
		break;
	    usleep(100 * 1000);
	}
	
	if (dead_pid == 0)
	{
	    kill(SIGKILL);
	    waitpid(pid, &status, 0);
	}
    }
}
