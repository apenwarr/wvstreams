/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A "Log Receiver" that logs to a file changing files every day
 *  the 'keep_for' variable controls the number of days to keep
 *  the old log file for. Setting it to 0 disables automatic
 *  purging of log files.
 */


#include <time.h>

#include "wvlogfile.h"
#include "wvtimeutils.h"
#include "wvdiriter.h"

///////////////////////////////////// WvLogFile

WvLogFile::WvLogFile(WvString _dirpath, WvString _basefname,
    int _keep_for, WvLog::LogLevel _max_level) : 
    WvDailyEvent(0, 1), WvLogRcv(_max_level), keep_for(_keep_for),
    dirpath(_dirpath), basefname(_basefname)
{
    start_log();
}

void WvLogFile::_make_prefix()
{
    time_t timenow;
    time(&timenow);
    struct tm* tmstamp = localtime(&timenow);
    char timestr[30];
    strftime(&timestr[0], 30, "%b %d %T %Z", tmstamp);
     
    prefix = WvString("%s: %s<%s>: ", timestr, appname(last_source),
        loglevels[last_level]);
    prelen = strlen(prefix);
}

void WvLogFile::_mid_line(const char *str, size_t len)
{
    logfile.write(str, len);
}

void WvLogFile::execute()
{
    WvDailyEvent::execute();
    // It's tomorrow. Move logging to a new file.
    logfile.close();
    start_log();

    // Look for old logs and purge them
    WvDirIter i(dirpath, false);
    i.rewind();
    while (i.next() && keep_for)
    {
        // if it begins with the base name
        if (!strncmp(i.ptr()->name, basefname, strlen(basefname)))
            // and it's older than 'keep_for' days
            if (i.ptr()->st_mtime <
                    wvtime().tv_sec - keep_for*86400) 
            {
                //delete it
                unlink(i.ptr()->fullname);
            }
    }
}

void WvLogFile::start_log()
{
    // Open the file and start logging to it
    time_t timenow = wvtime().tv_sec;
    struct tm* tmstamp = localtime(&timenow);
    char suffix[20];
    strftime(&suffix[0], 20, "%Y-%m-%d", tmstamp);
    WvString fullname("%s%s.%s", dirpath, basefname, suffix);
    logfile.open(fullname, O_WRONLY|O_APPEND|O_CREAT|O_SYNC, 0640);
}
