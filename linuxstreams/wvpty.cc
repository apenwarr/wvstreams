/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *  
 * WvStreams implementation of ptys under Linux.
 *
 * For more information on programming ptys, see chapter 19 of
 * Stevens' "Advanced Programming in the UNIX Environment"
 */

#include "wvpty.h"

#include <grp.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define DPRINTF(format, args...)
//#define DPRINTF(format, args...) fprintf(stderr, "WvPty:" format, ##args)

bool WvPty::open_master()
{
    const char *xvals = "pqrstuvwxyzPQRST";
    const char *yvals = "0123456789abcdef";
    char pty[] = "/dev/ptyXY";

    for (int i=0; xvals[i]; ++i)
    {
        pty[8] = xvals[i];

        for (int j=0; yvals[j]; ++j)
        {
            pty[9] = yvals[j];

            int fd = ::open(pty, O_RDWR);
            if (fd < 0)
            {
                if (errno == ENOENT)
                    return false; // no more ptys
            }
            else
            {
		DPRINTF("Chosen PTY is %s\n", pty);

                setfd(fd);

                _master = pty;
                _master.unique();

                pty[5] = 't';
                _slave = pty;
                _slave.unique();

                return true;
            }
        }
    }

    return false;
}

bool WvPty::open_slave()
{
    DPRINTF("Chosen TTY is %s\n", _slave.cstr());

    // try to change owner and permissions.  this will only work if we 
    // are root; if we're not root, we don't care.
    struct group *gr = ::getgrnam("tty");
    ::chown(_slave, ::getuid(), gr? gr->gr_gid: (gid_t)-1);

    // Workaround for bug 9900
    //::chmod(_slave, S_IRUSR | S_IWUSR | S_IWGRP);

    setfd(::open(_slave, O_RDWR));
    
    return getfd() != -1;
}

WvPty::WvPty(const char *program, const char * const *argv,
        Callback _pre_exec_cb, Callback _post_exec_cb)
        : _pid(-1), _exit_status(242),
          pre_exec_cb(_pre_exec_cb), post_exec_cb(_post_exec_cb)
{
    if (!open_master()
            || (_pid = ::fork()) < 0)
    {
        // error
        _pid = -1;
        setfd(-1);
    }
    else if (_pid == 0)
    {
        // child
        int fd = getfd();
        if (::close(fd) < 0)
        {
            DPRINTF("close(fd) failed: %s\n", strerror(errno));
            goto _error;
        }
        if (::setsid() < 0)
        {
            DPRINTF("setsid() failed: %s\n", strerror(errno));
            goto _error;
        }
        if (!open_slave())
        {
            DPRINTF("open_slave() failed: %s\n", strerror(errno));
            goto _error;
        }
        ::ioctl(getrfd(), TIOCSCTTY, NULL); // This may fail in case opening the 
                                            // ptys in open_slave proactively gave us a
                                            // controling terminal
        if (::dup2(getrfd(), STDIN_FILENO) < 0)
        {
            DPRINTF("dup2(0) failed: %s\n", strerror(errno));
            goto _error;
        }
        if (::dup2(getwfd(), STDOUT_FILENO) < 0)
        {
            DPRINTF("dup2(1) failed: %s\n", strerror(errno));
            goto _error;
        }
        if (::dup2(getwfd(), STDERR_FILENO) < 0)
        {
            DPRINTF("dup2(2) failed: %s\n", strerror(errno));
            goto _error;
        }
        if (getfd() > STDERR_FILENO && ::close(getfd()) < 0)
        {
            DPRINTF("close(getfd()) failed: %s\n", strerror(errno));
            goto _error;
        }
        
	if (::fcntl(STDIN_FILENO, F_SETFL,
	    	fcntl(STDIN_FILENO, F_GETFL) & (O_APPEND|O_ASYNC)))
	{
            DPRINTF("fcntl(0) failed: %s\n", strerror(errno));
            goto _error;
	}
	if (::fcntl(STDOUT_FILENO, F_SETFL,
	    	fcntl(STDOUT_FILENO, F_GETFL) & (O_APPEND|O_ASYNC)))
	{
            DPRINTF("fcntl(1) failed: %s\n", strerror(errno));
            goto _error;
	}
	if (::fcntl(STDERR_FILENO, F_SETFL,
	    	fcntl(STDERR_FILENO, F_GETFL) & (O_APPEND|O_ASYNC)))
	{
            DPRINTF("fcntl(2) failed: %s\n", strerror(errno));
            goto _error;
	}
       
        setfd(-1);
        if (pre_exec_cb && !pre_exec_cb(*this)) goto _error;
        execvp(program, (char * const *)argv);
        if (post_exec_cb) post_exec_cb(*this);

_error:
        setfd(-1);
        _exit(242);
    }
}

void WvPty::kill(int signum)
{
    if (_pid != -1)
	::kill(_pid, signum);
}

void WvPty::monitor_child(bool wait)
{
    if (_pid != -1)
    {
        int status;
        if (waitpid(_pid, &status, wait? 0: WNOHANG) == _pid)
        {
            _pid = -1;
            _exit_status = status;
        }
    }
}

bool WvPty::child_exited()
{
    monitor_child(false);
    return _pid == -1;     
}

bool WvPty::child_killed()
{
    monitor_child(false);
    return _pid == -1 && WIFSIGNALED(_exit_status);
}

int WvPty::finish()
{
    monitor_child(true);
    return WEXITSTATUS(_exit_status);
}

int WvPty::exit_status()
{
    monitor_child(false);
    if (_pid == -1)
    {
        if (child_killed())
            return WTERMSIG(_exit_status);
        else
            return WEXITSTATUS(_exit_status);
    }
    else
        return 242;
}

