/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvLogBuffer is a descendant of WvLogRcv that buffers log messages for
 * later use.  It only keeps up to max_lines log entries of max_level debug
 * level or lower.
 */
#ifndef __WVLOGBUFFER_H
#define __WVLOGBUFFER_H

#include "wvlogrcv.h"

class WvLogBuffer : public WvLogRcv
{
public:
    class Msg
    {
    public:
	time_t timestamp;
	WvLog::LogLevel level;
	WvString source, message;
	
	Msg(WvLog::LogLevel _level, const WvString &_source);
    };
    
    DeclareWvList(Msg);
    
protected:
    Msg *lastmsg;
    MsgList msgs;
    WvBuffer current;
    int max_lines;
    int numlines[WvLog::NUM_LOGLEVELS];
    
    virtual void _begin_line();
    virtual void _mid_line(const char *str, size_t len);
    virtual void _end_line();

public:
    WvLogBuffer(int _max_lines, 
		WvLog::LogLevel _max_level = WvLog::NUM_LOGLEVELS);
    virtual ~WvLogBuffer();

    MsgList &messages()
    	{ end_line(); return msgs; }
    void dump(WvStream &s);
};

#endif // __WVLOGBUFFER_H
