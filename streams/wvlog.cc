/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Functions needed to implement general WvLog class.
 * 
 * See wvlog.h for more information.
 */
#include "wvlogrcv.h"
#include "wvstringlist.h"
#include "strutils.h"
#include "wvfork.h"

#include <ctype.h>

#ifdef _WIN32
#include <io.h>
#define snprintf _snprintf
#endif

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



WvLog::WvLog(WvStringParm _app, LogLevel _loglevel, WvLogFilter* _filter)
    : app(_app), loglevel(_loglevel), filter(_filter)
{
//    printf("log: %s create\n", app.cstr());
    num_logs++;
    set_wsname(app);
}


WvLog::WvLog(const WvLog &l)
    : app(l.app), loglevel(l.loglevel), filter(l.filter)
{
//    printf("log: %s create\n", app.cstr());
    num_logs++;
    set_wsname(app);
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
//    printf("log: %s delete\n", app.cstr());
//    printf("num_logs is now %d\n", num_logs);
}


bool WvLog::isok() const
{
    return true;
}


void WvLog::pre_select(SelectInfo &si)
{
    // a wvlog is always writable...
    if (si.wants.writable)
	si.msec_timeout = 0;
    else
	WvStream::pre_select(si);
}


bool WvLog::post_select(SelectInfo &si)
{
    // a wvlog is always writable...
    if (si.wants.writable)
	return true;
    else
	return WvStream::post_select(si);
}


size_t WvLog::uwrite(const void *_buf, size_t len)
{
    // Writing the log message to a stream might cause it to emit its own log
    // messages, causing recursion.  Don't let it get out of hand.
    static const int recursion_max = 8;
    static int recursion_count = 0;
    static WvString recursion_msg("Too many extra log messages written while "
            "writing to the log.  Suppressing additional messages.\n");

    ++recursion_count;

    if (!num_receivers)
    {
	if (!default_receiver)
	{
	    // nobody's listening -- create a receiver on the console
	    int xfd = dup(2);
	    default_receiver = new WvLogConsole(xfd);
	    num_receivers--; // default does not qualify!
	}

        if (recursion_count < recursion_max)
            default_receiver->log(app, loglevel, (const char *)_buf, len);
        else if (recursion_count == recursion_max)
            default_receiver->log(app, WvLog::Warning, recursion_msg.cstr(),
                    recursion_msg.len());

        --recursion_count;
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
	WvLogRcvBase &rc = *i;

        if (recursion_count < recursion_max)
            rc.log(app, loglevel, (const char *)_buf, len);
        else if (recursion_count == recursion_max)
            rc.log(app, WvLog::Warning, recursion_msg.cstr(), 
                    recursion_msg.len());
    }
    
    --recursion_count;
    return len;
}



///////////////////////////////////// WvLogRcvBase



WvLogRcvBase::WvLogRcvBase()
{
    static_init();
    WvLogRcvBase::force_new_line = false;
    WvLog::receivers.append(this, false);
    WvLog::num_receivers++;
}


WvLogRcvBase::~WvLogRcvBase()
{
    WvLog::receivers.unlink(this);
    WvLog::num_receivers--;
}


const char *WvLogRcvBase::appname(WvStringParm log) const
{
    if (log)
	return log;
    else
	return "unknown";
}


void WvLogRcvBase::static_init()
{
    static bool init = false;
    if (!init)
    {
#ifndef _WIN32
        add_wvfork_callback(WvLogRcvBase::cleanup_on_fork);
#endif
        init = true;
    }
}


void WvLogRcvBase::cleanup_on_fork(pid_t p)
{
    if (p) return;      // parent: do nothing

    WvLog::receivers.zap();
    delete WvLog::default_receiver;
    WvLog::default_receiver = NULL;
    WvLog::num_receivers = 0;
}



//////////////////////////////////// WvLogRcv



WvLogRcv::WvLogRcv(WvLog::LogLevel _max_level) : custom_levels(5)
{
    last_source = WvString();
    last_level = WvLog::NUM_LOGLEVELS;
    last_time = 0;
    max_level = _max_level;
    at_newline = true;
}


WvLogRcv::~WvLogRcv()
{
}


void WvLogRcv::_make_prefix(time_t now)
{
    prefix = WvString("%s<%s>: ",
        last_source, loglevels[last_level]);
    prelen = prefix.len();
}


void WvLogRcv::_begin_line()
{
    mid_line(prefix, prelen);
}


void WvLogRcv::_end_line()
{
    // do nothing
}


// like isprint(), but always treats chars >128 as printable, because they
// always are (even if they're meaningless)
static bool my_isprint(char _c)
{
    unsigned char c = _c;
    if (isprint(c) || c >= 128)
	return true;
    else
	return false;
}


void WvLogRcv::log(WvStringParm source, int _loglevel,
			const char *_buf, size_t len)
{
    WvLog::LogLevel loglevel = (WvLog::LogLevel)_loglevel;
    char hex[5];
    WvLog::LogLevel threshold = max_level;
    WvString srcname(source);
    strlwr(srcname.edit());

    Src_LvlDict::Iter i(custom_levels);
    i.rewind(); 

    // Check if the debug level for the source has been overridden
    while (i.next())
    {
        if (strstr(srcname, i->src))
        {
            threshold = i->lvl;
            break;
        }
    }
     
    if (loglevel > threshold)
	return;

    // only need to start a new line with new headers if they headers have
    // changed.  if the source and level are the same as before, just continue
    // the previous log entry.
    time_t now = wvtime().tv_sec;
    if (source != last_source
            || loglevel != last_level
            || WvLogRcvBase::force_new_line)
    {
	end_line();
	last_source = source;
	last_level = loglevel;
        last_time = now;
        _make_prefix(now);
    }
    else if (last_time == 0 || now != last_time)
    {
	// ensure that even with the same source and level, logs will
	// properly get the right time associated with them. however,
	// don't split up log messages that should appear in a single
	// log line.
        last_time = now;
	if (at_newline)
	    _make_prefix(now);
    }
    
    const char *buf = (const char *)_buf, *bufend = buf + len, *cptr;

    // loop through the buffer, printing each character or its [hex] equivalent
    // if it is unprintable.  Also eat newlines unless they are appropriate.
    while (buf < bufend)
    {
	if (buf[0] == '\n' || buf[0] == '\r')
	{
	    end_line();
	    buf++;
	    continue;
	}

	begin_line();

	if (buf[0] == '\t')
	{
	    mid_line(" ", 1);
	    buf++;
	    continue;
	}
	else if (!my_isprint(buf[0]))
	{
	    snprintf(hex, 5, "[%02x]", buf[0]);
	    mid_line(hex, 4);
	    buf++;
	    continue;
	}

	// like strchr, but size-limited instead of null-terminated
	for (cptr = buf; cptr < bufend; cptr++)
	{
	    if (*cptr == '\n' || !my_isprint(*cptr))
		break;
	}
	
	if (cptr >= bufend) // end of buffer
	{
	    mid_line(buf, bufend - buf);
	    buf = bufend;
	}
	else if (*cptr == '\n') // end of line
	{
	    mid_line((const char *)buf, cptr - buf);
	    buf = cptr;
	}
	else // therefore (!my_isprint(*cptr))
	{
	    mid_line(buf, cptr - buf);
	    buf = cptr;
	}
    }
}

// input format: name=number, name=number, name=number, etc.
//    'name' is the name of a log service
//    'number' is the number of the log level to use.
bool WvLogRcv::set_custom_levels(WvString descr)
{
    custom_levels.zap();

    // Parse the filter line into individual rules
    WvStringList lst;
    WvStringList::Iter i(lst);
    lst.split(descr, ",= ");
    if (lst.isempty())
        return true;
    WvString src("");

    for (i.rewind(); i.next(); )
    {
        if (src != "")
        {
            if (atoi(*i) > 0 && atoi(*i) <= WvLog::NUM_LOGLEVELS)
            {
                custom_levels.add(new Src_Lvl(src, atoi(*i)), true);
                src = "";
            }
            else
                return false;
        }
        else
        {
            src = *i;
            strlwr(trim_string(src.edit()));
        }
    }
    if (src != "")
        return false;

    return true;
}


///////////////////////////////////// WvLogConsole



WvLogConsole::WvLogConsole(int _fd, WvLog::LogLevel _max_level) :
    WvFDStream(_fd), WvLogRcv(_max_level)
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
