/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes.  Handles
 * command-line argument processing, forking into the background,
 * and signal handling.
 */
#ifndef __WVDAEMON_H
#define __WVDAEMON_H

#include "wvautoconf.h"
#ifndef WITH_POPT
#error WvDaemon is only availible when WvStreams is compiled with popt support
#else

#include "wvstring.h"
#include "wvargs.h"
#include "wvlog.h"

class WvDaemon;

typedef WvCallback<void, WvDaemon &, void *> WvDaemonCallback;
    	
class WvDaemon
{
    
    public:

        WvString name;
        WvString version;

        WvArgs args;
        WvLog log;
    
    protected:

        WvDaemonCallback start_callback;
        WvDaemonCallback run_callback;
        WvDaemonCallback stop_callback;

    private:

        void *ud;

        volatile bool _want_to_die;
        volatile bool _want_to_restart;

        WvLog::LogLevel log_level;
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

    	WvDaemon(WvStringParm _name, WvStringParm _version,
                WvDaemonCallback _start_callback,
    	    	WvDaemonCallback _run_callback,
    	    	WvDaemonCallback _stop_callback,
                void *_ud = NULL);
    	
    	int run(const char *argv0); // returns exit status;
    	int run(int argc, char **argv); // returns exit status;

        void restart()
        {
            _want_to_restart = true;
        }
        void die()
        {
            _want_to_die = true;
        }

        bool want_to_restart() const
        {
            return _want_to_restart;
        }
        bool want_to_die() const
        {
            return _want_to_die;
        }

        bool should_run() const
        {
            return !_want_to_die && !_want_to_restart;
        }
};

#endif //WITH_POPT

#endif // __WVDAEMON_H
