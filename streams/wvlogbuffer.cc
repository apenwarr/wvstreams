/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * WvLogBuffer is a descendant of WvLogRcv that buffers log messages for
 * later use.  It only keeps up to max_lines log entries of _each_ log
 * level max_level or lower.
 */
#include "wvlogbuffer.h"
#include "strutils.h"
#include <time.h>

WvLogBuffer::Msg::Msg(WvLog::LogLevel _level, const WvString &_source)
		: level(_level), source(_source)
{
    time(&timestamp);
}


WvLogBuffer::WvLogBuffer(int _max_lines, WvLog::LogLevel _max_level)
	: WvLogRcv(_max_level)
{
    max_lines = _max_lines;
    memset(numlines, 0, sizeof(numlines));
    lastmsg = NULL;
}


WvLogBuffer::~WvLogBuffer()
{
    end_line();
}


void WvLogBuffer::_begin_line()
{
    lastmsg = new Msg(last_level, last_source->app);
}


void WvLogBuffer::_mid_line(const char *str, size_t len)
{
    current.put(str, len);
}


void WvLogBuffer::_end_line()
{
    if (lastmsg)
    {
	current.put("", 1);  // terminating NULL
	lastmsg->message = trim_string((char *)current.get(current.used()));
	lastmsg->message.unique();
	
	if (lastmsg->level < WvLog::NUM_LOGLEVELS)
	{
	    // when we add a log message to the buffer, we need to remove 
	    // some from the top of the buffer, if it is fills...
	    int &nl = numlines[lastmsg->level];
	    
	    MsgList::Iter i(msgs);
	    i.rewind(); i.next();
	    while (nl >= max_lines && i.cur())
	    {
		Msg &m = i;
		if (m.level == lastmsg->level)
		{
		    i.unlink();
		    nl--;
		}
		else
		    i.next();
	    }
	    
	    msgs.append(lastmsg, true);
	    nl++;
	}
	else
	    delete lastmsg;
	lastmsg = NULL;
	
    }
}


void WvLogBuffer::dump(WvStream &s)
{
    MsgList::Iter i(messages());
    
    for (i.rewind(); i.next(); )
    {
	Msg &m = i;
	s.print("%s %s<%s>: %s+\n",
		m.timestamp, m.source, loglevels[m.level], m.message);
    }
}
