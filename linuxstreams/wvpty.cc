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
                rfd = wfd = fd;

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
    // try to change owner and permissions.  this will only work if we 
    // are root; if we're not root, we don't care.
    struct group *gr = ::getgrnam("tty");
    if (gr)
    {
        ::chown(_slave, ::getuid(), gr->gr_gid);
    }
    else
        ::chown(_slave, ::getuid(), (gid_t)-1);

    ::chmod(_slave, S_IRUSR | S_IWUSR | S_IWGRP);

    rfd = wfd = ::open(_slave, O_RDWR);
    
    return rfd != -1;
}

WvPty::WvPty(const char *program, const char * const *argv)
        : _pid(-1), _exit_status(242)
{
    if (!open_master()) return;

    if (!open_master()
            || (_pid = ::fork()) < 0)
    {
        // error
        _pid = -1;
        rfd = wfd = -1;
    }
    else if (_pid == 0)
    {
        // child
        int fd = rfd;
        if (::close(fd) < 0
                || ::setsid() < 0
                || !open_slave()
                || ::ioctl(rfd, TIOCSCTTY, NULL)
                || ::dup2(rfd, STDIN_FILENO) < 0
                || ::dup2(wfd, STDOUT_FILENO) < 0
                || ::dup2(wfd, STDERR_FILENO) < 0
                || (rfd > STDERR_FILENO && ::close(rfd) < 0))
            goto _error;
        
        execvp(program, (char * const *)argv);

_error:
        rfd = wfd = -1;
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

