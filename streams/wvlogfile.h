/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A "Log Receiver" that logs to a file changing files every day
 *  the 'keep_for' variable controls the number of days to keep
 *  the old log file for. Setting it to 0 disables automatic
 *  purging of log files.
 */

#ifndef __WVLOGFILE_H
#define __WVLOGFILE_H

#include "wvdailyevent.h"
#include "wvfile.h"
#include "wvlogrcv.h"

// Keep

class WvLogFile : public WvLogRcv
{
    public:
        WvLogFile(WvString _dirpath, WvString _basefname,
            int _keep_for, WvLog::LogLevel _max_level);
        void set_debug_level(WvLog::LogLevel _max_level)
            { max_level = _max_level; };

    protected:
        virtual void _make_prefix(); 
        virtual void _mid_line(const char *str, size_t len);
        void start_log();
        int keep_for, last_day;
        WvString dirpath, basefname;
        WvFile logfile;
};

#endif // __WVLOGFILE_H
