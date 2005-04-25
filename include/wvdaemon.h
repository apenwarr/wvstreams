/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes.  Handles
 * command-line argument processing, forking into the background,
 * and signal handling.
 */
#ifndef __WVDAEMON_H
#define __WVDAEMON_H

#include "wvstring.h"
#include "wvargs.h"
#include "wvlog.h"

class WvDaemon;

typedef WvCallback<void, WvDaemon &, void *> WvDaemonCallback;
    	
/*!
@brief WvDaemon - High-level abstraction for creating daemon processes.

WvDaemon makes it easy to create daemon processes that support forking
into the background and detaching from terminals, management of the .pid
file and the log file, and handling of the SIGTERM|SIGINT|SIGQUIT
and SIGHUP signals.

By default, daemons implemented through WvDaemon provide the following
command line options:

-q|--quit: decrease the log level by one
-v|--verbose: increase the log level by one
-d|--daemonize: fork into the background.
-s|--syslog: write log entries to the syslog() facility
-V|--version: print the program name and version number and exit immediately

These default arguments can be changed or appended to through the public member
WvDaemon::args of type WvArgs.

By default, daemons run in the foreground for debugging purposes; you must
pass the -d parameter to force them into the background.

The actual functionality of WvDaemon is implemented through three protected
member callbacks:

WvDaemon::start_callback: Called when the daemon first runs or after
restarting due to SIGHUP
WvDaemon::run_callback: The main loop callback.  It should return if
it ever expects that the daemon should exit or restart, ie. after having called
WvDaemon::die() or WvDaemon::restart()
WvDaemon::stop_callback: Called when the daemon is exiting or right before
restarting due to SIGHUP

Sample usage:

@code
#include <wvstreams/wvdaemon.h>

void run(WvDaemon &daemon, void *)
{
    int i = 1;
    while (daemon.should_run())
    {
        wvout->print("Loop %s\n", i++);
        sleep(1);
        if (i == 17)
            daemon.die();  // Exit after 16 seconds
    }
}

int main(int argc, char **argv)
{
    WvDaemon daemon("Sample Daemon", "0.1",
            WvDaemonCallback(), run, WvDaemonCallback());

    return daemon.run(argc, argv);
}
@endcode
!*/
class WvDaemon
{
    
    public:

        //! The name and version of the daemon; used for -V and logging
        WvString name;
        WvString version;
        //! The path to the pid file to use for the daemon; defaults
        //! to /var/run/name.pid, where name is above
        WvString pid_file;

        //! The arguments the daemon accepts; the defaults are described
        //! above.
        WvArgs args;
        //! The daemon's log mechanism
        WvLog log;
        WvLog::LogLevel log_level;
        bool syslog;
    
    protected:

        //! See the class description
        WvDaemonCallback start_callback;
        WvDaemonCallback run_callback;
        WvDaemonCallback stop_callback;

    private:

        void *ud;

        volatile bool _want_to_die;
        volatile bool _want_to_restart;

        bool daemonize;

        void dec_log_level(void *)
        {
            if ((int)log_level > (int)WvLog::Critical)
                log_level = (WvLog::LogLevel)((int)log_level - 1);
        }

        void inc_log_level(void *)
        {
            if ((int)log_level < (int)WvLog::Debug5)
                log_level = (WvLog::LogLevel)((int)log_level + 1);
        }

        void display_version_and_exit(void *)
        {
            wvout->print("%s version %s\n", name, version);
            ::exit(0);
        }

        int _run(const char *argv0);

    public:

        //! Construct a new daemon; requires the name, version,
        //! and three callbacks for the functionality of the daemon
    	WvDaemon(WvStringParm _name, WvStringParm _version,
                WvDaemonCallback _start_callback,
    	    	WvDaemonCallback _run_callback,
    	    	WvDaemonCallback _stop_callback,
                void *_ud = NULL);
    	
    	//! Run the daemon with no argument processing.  Returns exit status.
    	int run(const char *argv0);
    	//! Run the daemon after doing argument processing. Returns exit status.
    	int run(int argc, char **argv);

        //! Force the daemon to restart as soon as the run callback exits
        void restart()
        {
            _want_to_restart = true;
        }
        //! Force the daemon to exit as soon as the run callback exits
        void die()
        {
            _want_to_die = true;
        }

        //! Whether the daemon will restart when the run callback exits
        bool want_to_restart() const
        {
            return _want_to_restart;
        }
        //! Whether the daemon will quit when the run callback exits
        bool want_to_die() const
        {
            return _want_to_die;
        }

        //! Whether the daemon should continue runnning
        bool should_run() const
        {
            return !_want_to_die && !_want_to_restart;
        }
};

#endif // __WVDAEMON_H
