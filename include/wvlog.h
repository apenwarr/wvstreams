/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A generic data-logger class with support for multiple receivers.  If
 * no WvLogRcv objects have been created (see wvlogrcv.h) the default is
 * to log to stderr.
 * 
 * WvLog supports partial- and multiple-line log messages.  For example,
 *        log.print("test ");
 *        log.print("string\nfoo");
 * will print:
 *        appname(lvl): test string
 *        appname(lvl): foo
 */
#ifndef __WVLOG_H
#define __WVLOG_H

#include "wvstream.h"
#include <errno.h>

class WvLog;

// a WvLogRcv registers itself with WvLog and prints, captures,
// or transmits log messages.
class WvLogRcvBase
{
    friend class WvLog;
protected:
    const char *appname(const WvLog *log) const;
    virtual void log(const WvLog *source, int loglevel,
		     const char *_buf, size_t len) = 0;
public:
    WvLogRcvBase();
    virtual ~WvLogRcvBase();
};


DeclareWvList(WvLogRcvBase);


/**
 * A WvLog stream accepts log messages from applications and forwards them
 * to all registered WvLogRcv's.
 */
class WvLog : public WvStream
{
    friend class WvLogRcvBase;
public:
    enum LogLevel {
	Critical = 0,
	Error,
	Warning,
	Notice,
	Info,
	Debug, Debug1=Debug,
	Debug2,
	Debug3,
	Debug4,
	Debug5,
	
	NUM_LOGLEVELS
    };
    WvString app;

protected:
    const WvLog *parent;
    LogLevel loglevel;
    static WvLogRcvBaseList receivers;
    static int num_receivers, num_logs;
    static WvLogRcvBase *default_receiver;

public:
    WvLog(WvStringParm _app, LogLevel _loglevel = Info,
	  const WvLog *par = NULL);
    WvLog(const WvLog &l);
    virtual ~WvLog();

    /** fd==-1, but this stream is always ok */
    virtual bool isok() const;
    
    /* always writable */
    virtual bool pre_select(SelectInfo &si);

    /**
     * change the loglevel.  This returns the object again, so you can
     * make convenient statements like log.lvl(WvLog::Warning).print(...)
     */
    WvLog &lvl(LogLevel _loglevel)
        { loglevel = _loglevel; return *this; }
    
    /** change the loglevel and then print a message. */
    size_t operator() (LogLevel _loglevel, WvStringParm s)
    { 
	LogLevel l = loglevel; 
	size_t x = lvl(_loglevel).write(s);
	lvl(l);
	return x;
    }
    
    /** change the loglevel and then print a formatted message */
    size_t operator() (LogLevel _loglevel, WVSTRING_FORMAT_DECL)
    { 
	LogLevel l = loglevel;
	size_t x = lvl(_loglevel).print(WVSTRING_FORMAT_CALL);
	lvl(l);
	return x;
    }
    
    /**
     * although these appear in WvStream, they need to be re-listed for
     * some reason.
     */
    size_t operator() (WvStringParm s)
        { return WvStream::operator()(s); }
    size_t operator() (WVSTRING_FORMAT_DECL)
        { return WvStream::operator()(WVSTRING_FORMAT_CALL); }
    
    /**
     * split off a new WvLog object with the requested loglevel.  This way
     * you can have log at two or more levels without having to retype
     * log.lvl(WvLog::blahblah) all the time.
     */
    WvLog split(LogLevel _loglevel) const
        { return WvLog(app, _loglevel, this); }
    
    /**
     * we override the unbuffered write function, so lines also include the
     * application and log level.
     */
    virtual size_t uwrite(const void *buf, size_t len);
    
    /** a useful substitute for the normal C perror() function */
    void perror(WvStringParm s)
        { print("%s: %s\n", s, strerror(errno)); }
};


#endif // __WVLOG_H
