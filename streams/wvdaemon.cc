/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes.  Handles
 * command-line argument processing, forking into the background,
 * and signal handling.
 */

#include "wvdaemon.h"

#include "wvlinklist.h"
#include "wvsyslog.h"
#include "wvcrash.h"
#include "wvfile.h"
#include "wvatomicfile.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

DeclareWvList(WvDaemon);
static WvDaemonList daemons;

static void sighup_handler(int signum)
{
    signal(signum, SIG_IGN);

    WvDaemonList::Iter i(daemons);
    for (i.rewind(); i.next(); )
    {
        i->log(WvLog::Notice, "Restarting on signal %s.\n", signum);
        i->restart();
    }
}

static void sigterm_handler(int signum)
{
    signal(signum, SIG_IGN);

    WvDaemonList::Iter i(daemons);
    for (i.rewind(); i.next(); )
    {
        i->log(WvLog::Notice, "Dying on signal %s.\n", signum);
        i->die();
    }
}

static void sigquit_handler(int signum)
{
    signal(signum, SIG_IGN);

    exit(1);
}

WvDaemon::WvDaemon(WvStringParm _name, WvStringParm _version,
        WvDaemonCallback _start_callback,
        WvDaemonCallback _run_callback,
        WvDaemonCallback _stop_callback,
        void *_ud)
    : name(_name), version(_version),
            pid_file("/var/run/%s.pid", _name),
            log(_name, WvLog::Debug),
            start_callback(_start_callback),
            run_callback(_run_callback),
            stop_callback(_stop_callback),
            ud(_ud),
            log_level(WvLog::Info),
            daemonize(false),
            syslog(false)
{
    args.add_option('q', "quiet",
            "Decrease log level (can be used multiple times)",
            WvArgs::NoArgCallback(this, &WvDaemon::dec_log_level));
    args.add_option('v', "verbose",
            "Increase log level (can be used multiple times)",
            WvArgs::NoArgCallback(this, &WvDaemon::inc_log_level));
    args.add_set_bool_option('d', "daemonize",
            "Fork into background and return (implies -s)", daemonize);
    args.add_set_bool_option('s', "syslog",
            "Write log entries to syslog", syslog);
    args.add_option('V', "version",
            "Display version and exit",
            WvArgs::NoArgCallback(this, &WvDaemon::display_version_and_exit));
}

int WvDaemon::run(const char *argv0)
{
    if (daemonize)
    {
        pid_t pid = ::fork();
        if (pid < 0)
        {
            wverr->print("Failed to fork daemon: %s\n",
                    strerror(errno));
            return 3;
        }
        else if (pid == 0)
        {
            setsid();
            pid = fork();
            if (pid < 0)
            {
                wverr->print("Failed to double-fork daemon: %s\n",
                        strerror(errno));
            }
            else if (pid == 0)
            {
                WvSyslog syslog(name, false);
                
                ::chdir("/");
                
                ::umask(0);
                
                int null_fd = ::open("/dev/null", O_RDWR);
                if (null_fd == -1)
                {
                    log(WvLog::Error, "Failed to open /dev/null: %s\n",
                            strerror(errno));
                    _exit(1);
                }
                
                if (null_fd != 0 && (::close(0)
                            || ::dup2(null_fd, 0) == -1)
                        || null_fd != 1 && (::close(1)
                                || ::dup2(null_fd, 1) == -1)
                        || null_fd != 2 && (::close(2)
                                || ::dup2(null_fd, 2) == -1)
                        || null_fd != 0 && null_fd != 1 && null_fd != 2
                                && ::close(null_fd))
                {
                    // Can no longer write to syslog...
                    log(WvLog::Error, "Failed to close/dup2(0,1,2): %s\n",
                            strerror(errno));
                    _exit(1);
                }
                
                _run(argv0);
            }

            _exit(0);
        }

        return 0;
    }
    else
    {
        WvLogConsole console_log(STDOUT_FILENO, log_level);
        if (syslog)
        {
            WvSyslog syslog(name, false);
            return _run(argv0);
        }
        else return _run(argv0);
    }
}

int WvDaemon::run(int argc, char **argv)
{
    if (!args.process(argc, argv))
        return 1;

    return run(argv[0]);
}

int WvDaemon::_run(const char *argv0)
{
    wvcrash_setup(argv0);

    if (!!pid_file)
    {
        // FIXME: this is racy!
        
        // First, make sure we aren't already running
        WvFile old_pid_fd(pid_file, O_RDONLY);
        if (old_pid_fd.isok())
        {
            WvString line = old_pid_fd.getline(0);
            if (!!line)
            {
                pid_t old_pid = line.num();
                if (old_pid > 0 && (kill(old_pid, 0) == 0 || errno == EPERM))
                {
                    log(WvLog::Error,
                            "%s is already running (pid %s); exiting\n",
                            name, old_pid);
                    return 1;
                }
            }
        }
        old_pid_fd.close();

        // Now write our new PID file
        WvAtomicFile pid_fd(pid_file, 0666);
        pid_fd.print("%s\n", getpid());
        if (!pid_fd.isok())
            log(WvLog::Warning, "Failed to write PID file %s: %s\n",
                    pid_file, pid_fd.errstr());
        pid_fd.close();
    }

    log(WvLog::Notice, "Starting\n");
    log(WvLog::Info, "%s version %s\n", name, version);

    daemons.append(this, false);
    
    if (daemonize)
        signal(SIGINT, SIG_IGN);
    else
        signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGQUIT, sigquit_handler);
    signal(SIGHUP, sighup_handler);

    _want_to_die = false;
    while (!want_to_die())
    {
        _want_to_restart = false;

        start_callback(*this, ud);

        run_callback(*this, ud);

        stop_callback(*this, ud);
    }

    daemons.unlink(this);
    if (daemons.count() == 0)
    {
        signal(SIGHUP, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
    }

    log(WvLog::Notice, "Exiting\n");
    
    if (!!pid_file)
        ::unlink(pid_file);

    return 0;
}

