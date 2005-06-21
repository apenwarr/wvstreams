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
#include <sys/wait.h>

#define MAX_LOGFILE_SZ	1024*1024*100	// 100 Megs


//----------------------------------- WvLogFileBase ------------------

WvLogFileBase::WvLogFileBase(WvStringParm _filename, WvLog::LogLevel _max_level)
    : WvLogRcv(_max_level),
      WvFile(_filename, O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE, 0644)
{
    // nothing special
}


WvLogFileBase::WvLogFileBase(WvLog::LogLevel _max_level) 
    : WvLogRcv(_max_level) 
{ 
    // nothing special
}


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

    prefix = WvString("%s: %s<%s>: ", timestr, last_source_appname,
        loglevels[last_level]);
    prelen = prefix.len();
}

//----------------------------------- WvLogFile ----------------------

WvLogFile::WvLogFile(WvStringParm _filename, WvLog::LogLevel _max_level,
     int _keep_for, bool _force_new_line) : WvLogFileBase(_max_level), keep_for(_keep_for),
					    filename(_filename)
{
    WvLogRcv::force_new_line = _force_new_line;
    start_log();
}

void WvLogFile::_make_prefix()
{
    time_t timenow = wvtime().tv_sec;
    struct tm *tmstamp = localtime(&timenow);
    struct stat statbuf;

    // Get the filesize
    if (fstat(getfd(), &statbuf) == -1)
        statbuf.st_size = 0;

    // Start a new log when it's a new day or the file is too big
    if (last_day != tmstamp->tm_yday || statbuf.st_size > MAX_LOGFILE_SZ)
        start_log();

    WvLogFileBase::_make_prefix();
}


void WvLogFile::start_log()
{
    WvFile::close();

    int num = 0;
    struct stat statbuf;
    time_t timenow = wvtime().tv_sec;
    struct tm* tmstamp = localtime(&timenow);
    last_day = tmstamp->tm_yday;
    char buf[20];
    WvString fullname;
    strftime(buf, 20, "%Y-%m-%d", tmstamp);

    // Get the next filename
    do
        fullname = WvString("%s.%s.%s", filename, buf, num++);
    while (stat(fullname, &statbuf) != -1 && statbuf.st_size >= MAX_LOGFILE_SZ);

    WvString curname("%s.current", filename);
    WvString base = getfilename(filename);

    WvFile::open(fullname, O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE, 0644);

    // Don't delete the file, unless it's a symlink!
    int sym = readlink(curname, buf, 20);
    if (sym > 0 || errno == ENOENT)
    {
        unlink(curname);
        symlink(getfilename(fullname), curname);
    }

    // We fork here because this can be really slow when the directory has (oh, say 32,000 files)
    pid_t forky = wvfork();
    if (!forky)
    {
	// In child
	if (!wvfork())
	{
	    // Child will Look for old logs and purge them
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
	    _exit(0);
	}
	_exit(0);
    }
    int status;
    waitpid(forky, &status, 0);
}
