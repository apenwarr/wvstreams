/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
 * 
 * WvSyslog is a descendant of WvLogRcv that sends messages to the syslogd
 * daemon.
 */
#ifndef __WVSYSLOG_H
#define __WVSYSLOG_H

#include "wvlogrcv.h"

class WvSyslog : public WvLogRcv
{
public:
    WvSyslog(const WvString &_prefix, bool _include_appname,
	     WvLog::LogLevel _max_level = WvLog::NUM_LOGLEVELS);
    virtual ~WvSyslog();

protected:
    WvBuffer current;
    WvString syslog_prefix;
    bool include_appname;
    
    virtual void _begin_line();
    virtual void _mid_line(const char *str, size_t len);
    virtual void _end_line();
};

#endif // __WVLOGBUFFER_H
