/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of a WvPipe stream.  WvPipes allow you to create a new
 * process, attaching its stdin/stdout to a WvStream.
 * 
 * See wvpipe.h for more information.
 */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "wvpipe.h"
#include "wvsplitstream.h"

// The assorted WvPipe::WvPipe() constructors are described in wvpipe.h

WvPipe::WvPipe(const char *program, const char * const *argv,
	       bool writable, bool readable, bool catch_stderr,
	       int stdin_fd, int stdout_fd, int stderr_fd)
{
    setup(program, argv, writable, readable, catch_stderr,
	  stdin_fd, stdout_fd, stderr_fd);
}


WvPipe::WvPipe(const char *program, const char * const *argv,
	       bool writable, bool readable, bool catch_stderr,
	       WvStream *stdin_str, WvStream *stdout_str,
	       WvStream *stderr_str)
{
    int fd0 = 0, fd1 = 1, fd2 = 2;
    if (stdin_str)
	fd0 = stdin_str->getrfd();
    if (stdout_str)
	fd1 = stdout_str->getwfd();
    if (stderr_str)
	fd2 = stderr_str->getwfd();
    setup(program, argv, writable, readable, catch_stderr, fd0, fd1, fd2);
}


WvPipe::WvPipe(const char *program, const char **argv,
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


void WvPipe::setup(const char *program, const char * const *argv,
	      bool writable, bool readable, bool catch_stderr,
	      int stdin_fd, int stdout_fd, int stderr_fd)
{
    int socks[2];
    int flags;
    int waitfd;
    int pid;

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

    fcntl(socks[0], F_SETFL, O_RDWR|O_NONBLOCK);
    rwfd = socks[0];

    pid = proc.fork(&waitfd);

    if (!pid)
    {
	// child process
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

	/* never close stdin/stdout/stderr */
	fcntl(0, F_SETFD, 0);
	fcntl(1, F_SETFD, 0);
	fcntl(2, F_SETFD, 0);

	/* drop the O_NONBLOCK from stdin/stdout/stderr, it confuses
	 * some programs */
	flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, flags & (O_APPEND|O_ASYNC));
	flags = fcntl(1, F_GETFL);
	fcntl(1, F_SETFL, flags & (O_APPEND|O_ASYNC));
	flags = fcntl(2, F_GETFL);
	fcntl(2, F_SETFL, flags & (O_APPEND|O_ASYNC));

	/* If we're not capturing any of these through the socket, it
	 * means that the child end of the socket will be closed right
	 * at the execvp, which is bad. If we set the close-on-exec to
	 * false, the child end of the socket will be closed when the
	 * child (or sub-) process exits. */
	if (!writable && !readable && !catch_stderr)
	    fcntl(socks[1], F_SETFD, 0);  // never close the socketpair
	
	// this will often fail, but when it does work it is probably
	// the Right Thing To Do (tm)
	if (!readable && stdout_fd != 1)
	{
	    setsid();
	    ioctl(1, TIOCSCTTY, 1);
	}

	::close(waitfd);
	
	// now run the program.  If it fails, use _exit() so no destructors
	// get called and make a mess.
	execvp(program, (char * const *)argv);
	_exit(242);
    }
    else if (pid > 0)
    {
	// parent process
	::close(socks[1]);
    }
    else
    {
	::close(socks[0]);
	::close(socks[1]);
	return;
    }
}


// send the child process a signal
void WvPipe::kill(int signum)
{
    if (proc.running)
	proc.kill(signum);
}


// wait for the child to die
int WvPipe::finish()
{
    while (proc.running)
	proc.wait(100);
    
    return proc.estatus;
}


bool WvPipe::child_exited()
{
    /* FIXME: bug in WvSubProc? */
    proc.wait(0);
    proc.wait(0);
    return !proc.running;
}


// if child_exited(), return true if it died because of a signal, or
// false if it died due to a call to exit().
bool WvPipe::child_killed() const
{
    int st = proc.estatus;
    assert (WIFEXITED(st) || WIFSIGNALED(st));
    return WIFSIGNALED(st);
}


// return the numeric exit status of the child (if it exited) or the
// signal that killed the child (if it was killed).
int WvPipe::exit_status() const
{
    int st = proc.estatus;
    assert (WIFEXITED(st) || WIFSIGNALED(st));
    if (child_killed())
	return WTERMSIG(st);
    else
	return WEXITSTATUS(st);
}


WvPipe::~WvPipe()
{
    close();
}
