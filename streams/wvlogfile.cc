/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A "Log Receiver" that logs messages to a file
 */


#include <time.h>
#include "wvlogfile.h"
#include "wvtimeutils.h"
#include "wvdiriter.h"
#include "strutils.h"


//----------------------------------- WvLogFileBase ------------------

void WvLogFileBase::_mid_line(const char *str, size_t len)
{
    WvFile::write(str, len);
}

void WvLogFileBase::_make_prefix()
{
    time_t timenow = wvtime().tv_sec;
    struct tm* tmstamp = localtime(&timenow);
    char timestr[30];
    strftime(&timestr[0], 30, "%b %d %T %Z", tmstamp);
     
    prefix = WvString("%s: %s<%s>: ", timestr, appname(last_source),
        loglevels[last_level]);
    prelen = strlen(prefix);
}

//----------------------------------- WvLogFile ----------------------

WvLogFile::WvLogFile(WvStringParm _filename, WvLog::LogLevel _max_level,
     int _keep_for) : WvLogFileBase(_max_level), keep_for(_keep_for),
     filename(_filename)
{
    start_log();
}

void WvLogFile::_make_prefix()
{
    time_t timenow = wvtime().tv_sec;

    // Check if it's tomorrow yet, and start logging to a different file
    if (last_day != timenow/86400)
        start_log();
    
    WvLogFileBase::_make_prefix();
}


void WvLogFile::start_log()
{
    WvFile::close();

    time_t timenow = wvtime().tv_sec;
    last_day = timenow/86400;
    struct tm* tmstamp = localtime(&timenow);
    char buf[20];
    strftime(buf, 20, "%Y-%m-%d", tmstamp);
    WvString fullname("%s.%s", filename, buf);
    WvString curname("%s.current", filename);
    WvString base = getfilename(filename);

    WvFile::open(fullname, O_WRONLY|O_APPEND|O_CREAT, 0644);

    // Don't delete the file, unless it's a symlink!
    int sym = readlink(curname, buf, 20);
    if (sym > 0 || errno == ENOENT)
    {
        unlink(curname);
        symlink(fullname, curname);
    }

    // Look for old logs and purge them
    WvDirIter i(getdirname(filename), false);
    i.rewind();
    while (i.next() && keep_for)
    {
        // if it begins with the base name
        if (!strncmp(i.ptr()->name, base, strlen(base)))
            // and it's older than 'keep_for' days
            if (i.ptr()->st_mtime <
                    wvtime().tv_sec - keep_for*86400) 
            {
                //delete it
                unlink(i.ptr()->fullname);
            }
    }
}
