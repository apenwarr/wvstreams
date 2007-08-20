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
#ifndef _WIN32
#include "wvcrash.h"
#include "wvcrashlog.h"
#include "wvfile.h"
#include "wvatomicfile.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#include "wvlogrcv.h"
#endif

#ifndef _WIN32
# define CAN_SYSLOG true
# define CAN_DAEMONIZE true
#else
# define CAN_SYSLOG false
# define CAN_DAEMONIZE false
#endif

#ifdef _MSC_VER
static const int STDOUT_FILENO = 0;
#endif


WvDaemon *WvDaemon::singleton = NULL;


#ifndef _WIN32

static void sighup_handler(int signum)
{
    signal(signum, SIG_IGN);

    WvDaemon::me()->log(WvLog::Notice, "Restarting on signal %s.\n", signum);
    WvDaemon::me()->restart();
}


static void sigterm_handler(int signum)
{
    signal(signum, SIG_DFL);

    WvDaemon::me()->log(WvLog::Notice, "Dying on signal %s.\n", signum);
    WvDaemon::me()->die();
}


static void sigquit_handler(int signum)
{
    signal(signum, SIG_IGN);

    exit(1);
}

#endif // _WIN32

void WvDaemon::init(WvStringParm _name,
        WvStringParm _version,
        WvDaemonCallback _start_callback,
        WvDaemonCallback _run_callback,
        WvDaemonCallback _stop_callback,
        void *_ud)
{
    name = _name;
    version = _version;
    pid_file = WvString("/var/run/%s.pid", _name);
    daemonize = false;
    log_level = WvLog::Info;
    syslog = false;
    start_callback = _start_callback;
    run_callback = _run_callback;
    stop_callback = _stop_callback;
    ud = _ud;

    assert(singleton == NULL);
    singleton = this;
    
    args.add_option('q', "quiet",
            "Decrease log level (can be used multiple times)",
            WvArgs::NoArgCallback(this, &WvDaemon::dec_log_level));
    args.add_option('v', "verbose",
            "Increase log level (can be used multiple times)",
            WvArgs::NoArgCallback(this, &WvDaemon::inc_log_level));
    if (CAN_DAEMONIZE)
	args.add_option('d', "daemonize",
		"Fork into background and return (implies --syslog)",
		WvArgs::NoArgCallback(this, &WvDaemon::set_daemonize));
    if (CAN_SYSLOG)
    {
	args.add_set_bool_option('s', "syslog",
		 "Write log entries to syslog", syslog);
	args.add_reset_bool_option(0, "no-syslog",
		   "Do not write log entries to syslog", syslog);
    }
    
    args.set_version(WvString("%s version %s", name, version).cstr());
}


WvDaemon::~WvDaemon()
{
}


int WvDaemon::run(const char *argv0)
{
#ifndef _WIN32
    if (CAN_DAEMONIZE && daemonize)
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
                ::chdir("/");
                ::umask(0);
                
                int null_fd;
                do
                {
                    null_fd = ::open("/dev/null", O_RDWR);
                    if (null_fd == -1)
                    {
                        log(WvLog::Error, "Failed to open /dev/null: %s\n",
                                strerror(errno));
                        _exit(1);
                    }
                } while (null_fd == 0 || null_fd == 1 || null_fd == 2);
                
                if (::dup2(null_fd, 0) == -1
                        || ::dup2(null_fd, 1) == -1
                        || ::dup2(null_fd, 2) == -1)
                {
                    log(WvLog::Error, "Failed to dup2(null_fd, (0|1|2)): %s\n",
                            strerror(errno));
                    _exit(1);
                }
                ::close(null_fd);

                // Make sure the close-on-exec flag is not set for
                // the first three descriptors, since many programs
                // assume that they are open after exec()
                if (::fcntl(0, F_SETFD, 0) == -1
                        || ::fcntl(1, F_SETFD, 0) == -1
                        || ::fcntl(2, F_SETFD, 0) == -1)
                {
                    log(WvLog::Warning, "Failed to fcntl((0|1|2), F_SETFD, 0): %s\n",
                            strerror(errno));
                }
                
                return _run(argv0); // Make sure destructors are called
            }

            _exit(0);
        }

        return 0;
    }
    else
#endif // !_WIN32
    {
        WvLogConsole console_log(STDOUT_FILENO, log_level);
        if (CAN_SYSLOG && syslog)
        {
            WvSyslog syslog(name, false);
            return _run(argv0);
        }
        else 
	    return _run(argv0);
    }
}


int WvDaemon::run(int argc, char **argv)
{
    if (!args.process(argc, argv, &_extra_args))
        return 1;
    return run(argv[0]);
}


int WvDaemon::_run(const char *argv0)
{
    WvLogRcv *logr = NULL;
#ifndef _WIN32
    WvCrashLog crashlog;
    wvcrash_setup(argv0, version);
#endif
    
    if (CAN_SYSLOG && syslog)
	logr = new WvSyslog(name, false);

    _want_to_die = false;
    do_load();
    while (!want_to_die())
    {
        _want_to_restart = false;

        do_start();
        
        while (should_run())
            do_run();
        
        do_stop();
    }
    do_unload();
    
    if (logr)
	delete logr;

    return _exit_status;
}


void WvDaemon::do_load()
{
#ifndef _WIN32
    if (!!pid_file && daemonize)
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
                    die();
                }
            }
        }
        old_pid_fd.close();
        if (want_to_die())
            return;

        // Now write our new PID file
        WvAtomicFile pid_fd(pid_file, O_WRONLY, 0600);
        pid_fd.print("%s\n", getpid());
        if (!pid_fd.isok())
            log(WvLog::Warning, "Failed to write PID file %s: %s\n",
                    pid_file, pid_fd.errstr());
        pid_fd.close();
    }
#endif
    log(WvLog::Notice, "Starting %s version %s.\n", name, version);

#ifndef _WIN32    
    if (daemonize)
        signal(SIGINT, SIG_IGN);
    else
        signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGQUIT, sigquit_handler);
    signal(SIGHUP, sighup_handler);
#endif

    if (!!load_callback)
        load_callback(*this, ud);
}


void WvDaemon::do_start()
{
    if (!!start_callback)
        start_callback(*this, ud);
}


void WvDaemon::do_run()
{
    if (!!run_callback)
        run_callback(*this, ud);
}


void WvDaemon::do_stop()
{
    if (!!stop_callback)
        stop_callback(*this, ud);
}


void WvDaemon::do_unload()
{
    if (!!unload_callback)
        unload_callback(*this, ud);

#ifndef _WIN32
    signal(SIGHUP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
#endif

    log(WvLog::Notice, "Exiting with status %s\n", _exit_status);

#ifndef _WIN32    
    if (!!pid_file && daemonize)
        ::unlink(pid_file);
#endif
}


bool WvDaemon::set_daemonize(void *)
{
    daemonize = true;
    syslog = true;
    return true;
}
