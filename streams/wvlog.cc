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



WvLog::WvLog(WvStringParm _app, LogLevel _loglevel, const WvLog *par)
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


bool WvLog::pre_select(SelectInfo &si)
{
    // a wvlog is always writable...
    if (si.wants.writable)
	return true;
    else
	return WvStream::pre_select(si);
}


size_t WvLog::uwrite(const void *_buf, size_t len)
{
    if (!num_receivers)
    {
	if (!default_receiver)
	{
	    // nobody's listening -- create a receiver on the console
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
	WvLogRcvBase &rc = *i;
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
    return log->app;
}



//////////////////////////////////// WvLogRcv



WvLogRcv::WvLogRcv(WvLog::LogLevel _max_level) : custom_levels(5)
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


void WvLogRcv::log(const WvLog *source, int _loglevel,
			const char *_buf, size_t len)
{
    WvLog::LogLevel loglevel = (WvLog::LogLevel)_loglevel;
    char hex[5];
    WvLog::LogLevel threshold = max_level;
    WvString srcname = source->app;
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
    if (source != last_source || loglevel != last_level)
    {
	end_line();
	last_source = source;
	last_level = loglevel;
	_make_prefix();
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
	
	if (*cptr == '\n') // end of line
	{
	    mid_line((const char *)buf, cptr - buf);
	    buf = cptr;
	}
	else if (!my_isprint(*cptr))
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

bool WvLogRcv::set_custom_levels(WvString descr)
{
    custom_levels.zap();

    // Parse the filter line into individual rules
    WvStringList lst;
    WvStringList::Iter i(lst);
    lst.split(descr, ",=");
    if (!lst.count())
        return true;
    i.rewind();
    WvString src("");

    while(i.next())
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
