/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvLogBuffer is a descendant of WvLogRcv that buffers log messages for
 * later use.  It only keeps up to max_lines log entries for every
 * source/debug level, s.t. debug level <= max_level
 */                                                    
#include "wvlogbuffer.h"
#include "strutils.h"
#include <time.h>

WvLogBuffer::Msg::Msg(WvLog::LogLevel _level, WvStringParm _source,
    WvString _message) : level(_level), source(_source), message(_message)
{
    time(&timestamp);
}

WvLogBuffer::Msg* WvLogBuffer::MsgCounter::add(WvLogBuffer::Msg* msg, int max)
{
    list.append(msg, false);
    
    // Check if we need to purge anything
    if (list.count() > (size_t)max)
    {
        Msg* killme = list.first();
        list.unlink_first();
        return killme; 
    }
    else 
        return NULL;
}


WvLogBuffer::WvLogBuffer(int _max_lines, WvLog::LogLevel _max_level) :
    WvLogRcv(_max_level), counters(25)
{
    max_lines = _max_lines;
}


WvLogBuffer::~WvLogBuffer()
{
    end_line();
}


void WvLogBuffer::_mid_line(const char *str, size_t len)
{
    current.put(str, len);
}


void WvLogBuffer::_end_line()
{
    if (last_level < WvLog::NUM_LOGLEVELS)
    {
        current.put("", 1);  // terminating NULL
        Msg *lastmsg = new Msg(last_level, last_source->app,
            trim_string((char *)current.get(current.used())));
        
        // Stick the msg in the list of all messages
        msgs.append(lastmsg, true);
        
        // Check if we already have any messages of this source/level
        WvString type(WvString("%s:%s", last_source->app, last_level));
        MsgCounter* msgcounter = counters[type];
        // If not create a new tracking list for it
        if (!msgcounter)
        {
            msgcounter = new MsgCounter(type);
            counters.add(msgcounter, true);
        }
        // Now that we are sure the type exists, add the message to it
        Msg* killme = msgcounter->add(lastmsg, max_lines);
        
        // Delete the extra messages if we need to
        if (killme)
            msgs.unlink(killme);
    }
    else 
        current.zap();
}


void WvLogBuffer::dump(WvStream &s)
{
    MsgList::Iter i(messages());
    
    for (i.rewind(); i.next(); )
    {
	Msg &m = *i;
	s.print("%s %s<%s>: %s+\n",
		m.timestamp, m.source, loglevels[m.level], m.message);
    }
}
