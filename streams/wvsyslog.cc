/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvSyslog is a descendant of WvLogRcv that sends messages to the syslogd
 * daemon.
 */
#include "wvsyslog.h"
#include "strutils.h"
#include <time.h>
#include <syslog.h>

WvSyslog::WvSyslog(const WvString &_prefix, bool _include_appname,
		   WvLog::LogLevel _max_level)
	: WvLogRcv(_max_level), syslog_prefix(_prefix)
{
    include_appname = _include_appname;
    openlog(syslog_prefix.str, 0, LOG_DAEMON);
}


WvSyslog::~WvSyslog()
{
    end_line();
    closelog();
}


void WvSyslog::_begin_line()
{
    if (include_appname)
	current.put(prefix.str, prelen);
}


void WvSyslog::_mid_line(const char *str, size_t len)
{
    current.put(str, len);
}


void WvSyslog::_end_line()
{
    int lev, count;
    struct LevMap
    {
	int wvlog_lvl;
	int syslog_lvl;
    };

    static LevMap levmap[] = {
	{WvLog::Critical,	LOG_CRIT},
	{WvLog::Error,		LOG_ERR},
	{WvLog::Warning,	LOG_WARNING},
	{WvLog::Notice,		LOG_NOTICE},
	{WvLog::Info,		LOG_INFO},
	{WvLog::Debug,		LOG_DEBUG},
	{WvLog::Debug2,		-1},
	{-1, -1}
    };
    
    if (current.used())
    {
	lev = -1;
	
	for (count = 0; levmap[count].wvlog_lvl >= 0; count++)
	{
	    if (last_level >= levmap[count].wvlog_lvl)
		lev = levmap[count].syslog_lvl;
	}
	
	if (lev >= 0)
	{
	    current.put("", 1); // null-terminate
	    syslog(lev, "%s", current.get(current.used()));
	}
	else
	    current.zap(); // not important enough to print this message
    }
}

