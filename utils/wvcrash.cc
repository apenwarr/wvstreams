/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Routines to generate a stack backtrace automatically when a program
 * crashes.
 */
#include "wvcrash.h"
#include <signal.h>
#include <fcntl.h>
#include <string.h>

// FIXME: this file mostly only works in Linux
#ifdef __linux

# include <execinfo.h>
#include <unistd.h>

static const char *argv0 = "UNKNOWN";
static const char *desc = NULL;

// write a string 'str' to fd
static void wr(int fd, const char *str)
{
    write(fd, str, strlen(str));
}


// convert 'num' to a string and write it to fd.
static void wrn(int fd, int num)
{
    int tmp;
    char c;
    
    if (num < 0)
    {
	wr(fd, "-");
	num = -num;
    } 
    else if (num == 0)
    {
	wr(fd, "0");
	return;
    }
    
    tmp = 0;
    while (num > 0)
    {
	tmp *= 10;
	tmp += num%10;
	num /= 10;
    }
    
    while (tmp > 0)
    {
	c = '0' + (tmp%10);
	write(fd, &c, 1);
	tmp /= 10;
    }
}


void wvcrash_real(int sig, int fd)
{
    static void *trace[64];
    static char *signame = strsignal(sig);
    
    wr(fd, argv0);
    if (desc)
    {
	wr(fd, " (");
	wr(fd, desc);
	wr(fd, ")");
    }
    wr(fd, " dying on signal ");
    wrn(fd, sig);
    if (signame)
    {
	wr(fd, " (");
	wr(fd, signame);
	wr(fd, ")");
    }
    wr(fd, "\n\nBacktrace:\n");
    backtrace_symbols_fd(trace,
		 backtrace(trace, sizeof(trace)/sizeof(trace[0])), fd);
    
    // we want to create a coredump, and the kernel seems to not want to do
    // that if we send ourselves the same signal that we're already in.
    // Whatever... just send a different one :)
    if (sig == SIGABRT)
	sig = SIGBUS;
    else if (sig != 0)
	sig = SIGABRT;
    
    signal(sig, SIG_DFL);
    raise(sig);
}


// Hint: we can't do anything really difficult here, because the program is
// probably really confused.  So we should try to limit this to straight
// kernel syscalls (ie. don't fiddle with FILE* or streams or lists, just
// use straight file descriptors.)
// 
// We fork a subprogram to do the fancy stuff like sending email.
// 
void wvcrash(int sig)
{
    int fds[2];
    pid_t pid;
    
    signal(sig, SIG_DFL);
    wr(2, "\n\nwvcrash: crashing!\n");
    
    // close some fds, just in case the reason we're crashing is fd
    // exhaustion!  Otherwise we won't be able to create our pipe to a
    // subprocess.  Probably only closing two fds is possible, but the
    // subproc could get confused if all the fds are non-close-on-exec and
    // it needs to open a few files.
    // 
    // Don't close fd 0, 1, or 2, however, since those might be useful to
    // the child wvcrash script.  Also, let's skip 3 and 4, in case someone
    // uses them for something.  But don't close fd numbers that are *too*
    // big; if someone ulimits the number of fds we can use, and *that's*
    // why we're crashing, there's no guarantee that high fd numbers are in
    // use even if we've run out.
    for (int count = 5; count < 15; count++)
	close(count);
    
    if (pipe(fds))
	wvcrash_real(sig, 2); // just use stderr instead
    else
    {
	pid = fork();
	if (pid < 0)
	    wvcrash_real(sig, 2); // just use stderr instead
	else if (pid == 0) // child
	{
	    close(fds[1]);
	    dup2(fds[0], 0); // make stdin read from pipe
	    fcntl(0, F_SETFD, 0);
	    
	    execlp("wvcrash", "wvcrash", NULL);
	    
	    // if we get here, we couldn't exec wvcrash
	    wr(2, "wvcrash: can't exec wvcrash binary "
	       "- writing to wvcrash.txt!\n");
	    execlp("dd", "dd", "of=wvcrash.txt", NULL);
	    
	    wr(2, "wvcrash: can't exec dd to write to wvcrash.txt!\n");
	    _exit(127);
	}
	else if (pid > 0) // parent
	{
	    close(fds[0]);
	    wvcrash_real(sig, fds[1]);
	}
    }
    
    // child (usually)
    _exit(126);
}


void wvcrash_setup(const char *_argv0, const char *_desc)
{
    argv0 = _argv0;
    desc = _desc;
    signal(SIGSEGV, wvcrash);
    signal(SIGBUS,  wvcrash);
    signal(SIGABRT, wvcrash);
    signal(SIGFPE,  wvcrash);
    signal(SIGILL,  wvcrash);
}

#else // Not Linux

void wvcrash(int sig) {}
void wvcrash_setup(const char *_argv0, const char *_desc) {}

#endif // Not Linux
