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

bool WvPty::open_pty(WvString &master, int &master_fd,
    	WvString &slave, int &slave_fd)
{
    const char *xvals = "pqrstuvwxyzPQRST";
    const char *yvals = "0123456789abcdef";
    char pty[] = "/dev/ptyXY";
    char tty[] = "/dev/ttyXY";

    for (int i=0; xvals[i]; ++i)
    {
        pty[8] = tty[8] = xvals[i];
 
        for (int j=0; yvals[j]; ++j)
        {
            pty[9] = tty[9] = yvals[j];

            master_fd = ::open(pty, O_RDWR);
            if (master_fd >= 0)
	    	slave_fd = ::open(tty, O_RDWR);
	    else slave_fd = -1;
            if (master_fd < 0 || slave_fd < 0)
            {
		int saved_errno = errno;
		if (master_fd >= 0) ::close(master_fd);
		if (slave_fd >= 0) ::close(slave_fd);
                if (saved_errno == ENOENT)
                {
		    DPRINTF("No more PTYs (ENOENT)\n");
                    return false; // no more ptys
                }
            }
            else
            {
		DPRINTF("PTY is %s\n", (master = WvString(pty)).edit());
    	    	DPRINTF("TTY is %s\n", (slave = WvString(tty)).edit());

    	    	// try to change owner and permissions of slave.
    	    	// this will only work if we 
    	    	// are root; if we're not root, we don't care.
    	    	struct group *gr = ::getgrnam("tty");
    	    	::fchown(slave_fd, ::getuid(), gr? gr->gr_gid: (gid_t)-1);
    	    	::fchmod(slave_fd, S_IRUSR | S_IWUSR | S_IWGRP);

                return true;
            }
        }
    }

    DPRINTF("No more PTYs\n");
    return false;
}

WvPty::WvPty(const char *program, const char * const *argv,
        Callback _pre_exec_cb, Callback _post_exec_cb)
        : _pid(-1), _exit_status(242),
          pre_exec_cb(_pre_exec_cb), post_exec_cb(_post_exec_cb)
{
    int master_fd, slave_fd;
    if (!open_pty(_master, master_fd, _slave, slave_fd)
            || (_pid = ::fork()) < 0)
    {
        // error
        _pid = -1;
        setfd(-1);
    }
    else if (_pid == 0)
    {
        // child
        static const int std_fds[] = {
    	    STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, -1
    	};
    	const int *std_fd;
    	
        if (::close(master_fd) < 0)
        {
            DPRINTF("close(master_fd) failed: %s\n", strerror(errno));
            goto _error;
        }
        if (::setsid() < 0)
        {
            DPRINTF("setsid() failed: %s\n", strerror(errno));
            goto _error;
        }
        ::ioctl(slave_fd, TIOCSCTTY, NULL); // This may fail in case opening the 
                                            // ptys in open_slave proactively gave us a
                                            // controling terminal
        for (std_fd = std_fds; *std_fd != -1; ++std_fd)
        {
            if (::dup2(slave_fd, *std_fd) < 0)
            {
            	DPRINTF("dup2(slave_fd, %s) failed: %s\n", *std_fd,
            	    	strerror(errno));
            	goto _error;
            }
        }
        if (slave_fd > STDERR_FILENO && ::close(slave_fd) < 0)
        {
            DPRINTF("close(slave_fd) failed: %s\n", strerror(errno));
            goto _error;
        }
        
        for (std_fd = std_fds; *std_fd != -1; ++std_fd)
        {
	    if (::fcntl(*std_fd, F_SETFL,
	    	    fcntl(*std_fd, F_GETFL) & (O_APPEND|O_ASYNC)))
	    {
            	DPRINTF("fcntl(%s, F_SETFL) failed: %s\n", *std_fd,
            	    	strerror(errno));
            	goto _error;
	    }
	}
       
        if (pre_exec_cb && !pre_exec_cb(*this)) goto _error;
        execvp(program, (char * const *)argv);
        if (post_exec_cb) post_exec_cb(*this);

_error:
        _exit(242);
    }
    else
    {
        // parent
        if (::close(slave_fd) < 0)
        {
            DPRINTF("close(slave_fd) failed: %s\n", strerror(errno));
            goto _error;
        }
	setfd(master_fd);
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

