/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Functions needed to implement general WvLog class.
 * 
 * See wvlog.h for more information.
 */
#include "wvlogrcv.h"
#include <ctype.h>

WvLogRcvBaseList WvLog::receivers;
int WvLog::num_receivers = 0, WvLog::num_logs = 0;
WvLogRcvBase *WvLog::default_receiver = NULL;

char *WvLogRcv::loglevels[WvLog::NUM_LOGLEVELS] = {
    "Crit",
    "Err",
    "Warn",
    "Notice",
    "Info",
    "*1",
    "*2",
    "*3",
    "*4",
    "*5",
};



/////////////////////////////////////// WvLog



WvLog::WvLog(const WvString &_app, LogLevel _loglevel, const WvLog *par)
	: app(_app)
{
    parent = par;
    loglevel = _loglevel;
    num_logs++;
}


WvLog::WvLog(const WvLog &l)
{
    parent = l.parent ? l.parent : &l;
    app = parent->app;
    loglevel = parent->loglevel;
    num_logs++;
}


WvLog::~WvLog()
{
    num_logs--;
    if (!num_logs && default_receiver)
    {
	num_receivers++; // deleting default does not really reduce
	delete default_receiver;
	default_receiver = NULL;
    }
}


bool WvLog::isok() const
{
    return true;
}


size_t WvLog::uwrite(const void *_buf, size_t len)
{
    if (!num_receivers)
    {
	if (!default_receiver)
	{
	    default_receiver = new WvLogConsole(dup(2));
	    num_receivers--; // default does not qualify!
	}
	default_receiver->log(parent ? parent : this, loglevel,
			      (const char *)_buf, len);
	return len;
    }
    else if (default_receiver)
    {
	// no longer empty list -- delete our default to stderr
	num_receivers++; // deleting default does not really reduce
	delete default_receiver;
	default_receiver = NULL;
    }
    
    WvLogRcvBaseList::Iter i(receivers);
    for (i.rewind(); i.next(); )
    {
	WvLogRcvBase &rc = *i.data();
	rc.log(parent ? parent : this, loglevel, (const char *)_buf, len);
    }
    
    return len;
}



///////////////////////////////////// WvLogRcvBase



WvLogRcvBase::WvLogRcvBase()
{
    WvLog::receivers.append(this, false);
    WvLog::num_receivers++;
}


WvLogRcvBase::~WvLogRcvBase()
{
    WvLog::receivers.unlink(this);
    WvLog::num_receivers--;
}


const char *WvLogRcvBase::appname(const WvLog *log) const
{
    return log->app.str;
}



//////////////////////////////////// WvLogRcv



WvLogRcv::WvLogRcv(WvLog::LogLevel _max_level)
{
    last_source = NULL;
    last_level = WvLog::NUM_LOGLEVELS;
    max_level = _max_level;
    at_newline = true;
}


WvLogRcv::~WvLogRcv()
{
}


void WvLogRcv::_make_prefix()
{
    prefix = WvString("%s<%s>: ",
		      appname(last_source), loglevels[last_level]);
    prelen = strlen(prefix.str);
}


void WvLogRcv::_begin_line()
{
    mid_line(prefix.str, prelen);
}


void WvLogRcv::_end_line()
{
    // do nothing
}


void WvLogRcv::log(const WvLog *source, int _loglevel,
			const char *_buf, size_t len)
{
    WvLog::LogLevel loglevel = (WvLog::LogLevel)_loglevel;
    char hex[5];
    
    if (loglevel > max_level)
	return;
    
    if (source != last_source || loglevel != last_level)
    {
	end_line();
	last_source = source;
	last_level = loglevel;
	_make_prefix();
    }
    
    char *buf = (char *)_buf, *bufend = buf + len, *cptr;

    while (buf < bufend)
    {
	if (buf[0] == '\n' || buf[0] == '\r')
	{
	    end_line();
	    buf++;
	    continue;
	}

	begin_line();
	
	if (!isascii(buf[0]) || !isprint(buf[0]))
	{
	    snprintf(hex, 5, "[%02x]", *(unsigned char *)buf);
	    mid_line(hex, 4);
	    buf++;
	    continue;
	}

	// like strchr, but size-limited instead of null-terminated
	for (cptr = buf; cptr < bufend; cptr++)
	    if (*cptr == '\n' || !isascii(*cptr) || !isprint(*cptr)) break;
	
	if (*cptr == '\n') // end of line
	{
	    mid_line(buf, cptr - buf);
	    buf = cptr;
	}
	else if (!isascii(*cptr) || !isprint(*cptr))
	{
	    mid_line(buf, cptr - buf);
	    buf = cptr;
	}
	else // end of buffer
	{
	    mid_line(buf, bufend - buf);
	    buf = bufend;
	}
    }
}



///////////////////////////////////// WvLogConsole



WvLogConsole::WvLogConsole(int _fd, WvLog::LogLevel _max_level)
	: WvStream(_fd), WvLogRcv(_max_level)
{
}


WvLogConsole::~WvLogConsole()
{
    end_line();
}


void WvLogConsole::_mid_line(const char *str, size_t len)
{
    uwrite(str, len);
}
