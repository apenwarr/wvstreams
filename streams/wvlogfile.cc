/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A "Log Receiver" that logs messages to a file
 */
#include "wvlogfile.h"
#include "wvtimeutils.h"
#include "wvdiriter.h"
#include "strutils.h"
#include "wvdailyevent.h"
#include "wvfork.h"
#include <time.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif

#define MAX_LOGFILE_SZ	1024*1024*100	// 100 Megs

static time_t gmtoffset()
{
    time_t nowgmt = time(NULL);
    struct tm gmt = *gmtime(&nowgmt);
    struct tm local = *localtime(&nowgmt);
    time_t nowantilocal = mktime(&gmt); // mktime assumes gmt
    return nowgmt - nowantilocal;
}


//----------------------------------- WvLogFileBase ------------------

WvLogFileBase::WvLogFileBase(WvStringParm _filename, WvLog::LogLevel _max_level)
    : WvLogRcv(_max_level),
      WvFile(_filename, O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE, 0644)
{
    fsync_every = fsync_count = 0;
}


WvLogFileBase::WvLogFileBase(WvLog::LogLevel _max_level) 
    : WvLogRcv(_max_level) 
{ 
    fsync_every = fsync_count = 0;
}


void WvLogFileBase::_mid_line(const char *str, size_t len)
{
    WvFile::write(str, len);
}


void WvLogFileBase::_end_line()
{
    if (fsync_every)
    {
        fsync_count--;
        if (fsync_count <= 0 || fsync_count > fsync_every)
        {
            fsync_count = fsync_every;
            //WvFile::print("tick!\n");
            WvFile::flush(1000);
            fsync(getwfd());
        }
    }
}

#ifdef _WIN32
#define TIME_FORMAT "%b %d %H:%M:%S" // timezones in win32 look stupid
#else
#define TIME_FORMAT "%b %d %H:%M:%S %Z"
#endif

void WvLogFileBase::_make_prefix()
{
    time_t timenow = wvtime().tv_sec;
    struct tm* tmstamp = localtime(&timenow);
    char timestr[30];
    strftime(&timestr[0], 30, TIME_FORMAT, tmstamp);

    prefix = WvString("%s: %s<%s>: ", timestr, appname(last_source),
        loglevels[last_level]);
    prelen = prefix.len();
}

//----------------------------------- WvLogFile ----------------------

WvLogFile::WvLogFile(WvStringParm _filename, WvLog::LogLevel _max_level,
                     int _keep_for, bool _force_new_line, bool _allow_append)
    : WvLogFileBase(_max_level), keep_for(_keep_for), filename(_filename),
      allow_append(_allow_append)
{
    WvLogRcv::force_new_line = _force_new_line;
    start_log();
}

void WvLogFile::_make_prefix()
{
    time_t timenow = wvtime().tv_sec;
    // struct tm *tmstamp = localtime(&timenow);
    struct stat statbuf;

    // Get the filesize
    if (fstat(getfd(), &statbuf) == -1)
        statbuf.st_size = 0;

    // Make sure we are calculating last_day in the current time zone.
    if (last_day < ((timenow + gmtoffset())/86400) 
	|| statbuf.st_size > MAX_LOGFILE_SZ)
        start_log();

    WvLogFileBase::_make_prefix();
}

static void trim_old_logs(WvStringParm filename, WvStringParm base,
			  int keep_for)
{
    if (!keep_for) return;
    WvDirIter i(getdirname(filename), false);
    for (i.rewind(); i.next(); )
    {
	// if it begins with the base name
	if (!strncmp(i.ptr()->name, base, strlen(base)))
	{
	    // and it's older than 'keep_for' days
	    if (i.ptr()->st_mtime < wvtime().tv_sec - keep_for*86400)
		::unlink(i.ptr()->fullname);
	}
    }
}


void WvLogFile::start_log()
{
    WvFile::close();

    int num = 0;
    struct stat statbuf;
    time_t timenow = wvtime().tv_sec;
    last_day = (timenow + gmtoffset()) / 86400;
    struct tm* tmstamp = localtime(&timenow);
    char buf[20];
    WvString fullname;
    strftime(buf, 20, "%Y-%m-%d", tmstamp);

    // Get the next filename
    do
        fullname = WvString("%s.%s.%s", filename, buf, num++);
    while (stat(fullname, &statbuf) != -1
                && (statbuf.st_size >= MAX_LOGFILE_SZ || !allow_append));

    WvString curname("%s.current", filename);
    WvString base = getfilename(filename);

    WvFile::open(fullname, O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE, 0644);

#ifndef _WIN32 // no symlinks in win32
    // Don't delete the file, unless it's a symlink!
    int sym = readlink(curname, buf, 20);
    if (sym > 0 || errno == ENOENT)
    {
        unlink(curname);
        symlink(getfilename(fullname), curname);
    }
#endif

#ifndef _WIN32
    // We fork here because this can be really slow when the directory has
    // (oh, say 32,000 files)
    pid_t forky = wvfork();
    if (!forky)
    {
	// Child will Look for old logs and purge them
	trim_old_logs(filename, base, keep_for);
	_exit(0);
    }
#else
    // just do it in the foreground on Windows
    trim_old_logs(filename, base, keep_for);
#endif
}
