/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes that do
 * nothing but listen on a list of WvStreams and add connections
 * to the global list.
 */

#include "wvstreamsdaemon.h"

#ifndef _WIN32
#include <signal.h>
#endif

void WvStreamsDaemon::init(WvDaemonCallback cb)
{
    do_full_close = false;
    setcallback(cb);
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
}

void WvStreamsDaemon::do_start()
{
    WvDaemon::do_start();
    
    callback();
}

void WvStreamsDaemon::do_run()
{
    if (streams.isempty())
    {
        log(WvLog::Error, "No streams; exiting\n");
        die();
    }

    while (should_run())
    {
        WvDaemon::do_run();
        WvIStreamList::globallist.runonce();
    }
}

void WvStreamsDaemon::do_stop()
{
    WvIStreamList::Iter stream(streams);
    for (stream.rewind(); stream.next(); )
        WvIStreamList::globallist.unlink(stream.ptr());
    streams.zap();
    if (do_full_close || want_to_die())
        WvIStreamList::globallist.zap();
    
    WvDaemon::do_stop();
}

void WvStreamsDaemon::add_stream(IWvStream *istream,
    	bool autofree, char *id)
{
    streams.append(istream, false, id);
    // FIXME: we should pass in "id" here, but things are not happy in
    // const-correctness-land.
    WvIStreamList::globallist.append(istream, autofree, id);
}

void WvStreamsDaemon::add_restart_stream(IWvStream *istream, bool autofree,
					 char *id)
{
    add_stream(istream, autofree, id);
    
    istream->setclosecallback(wv::bind(&WvStreamsDaemon::restart_close_cb,
				       this, istream, id));
}

void WvStreamsDaemon::add_die_stream(IWvStream *istream,
    	bool autofree, char *id)
{
    add_stream(istream, autofree, id);
    
    istream->setclosecallback(wv::bind(&WvStreamsDaemon::die_close_cb, this,
				       istream, id));
}

void WvStreamsDaemon::restart_close_cb(IWvStream *s, const char *id)
{
    if (should_run())
    {
	WvString err = s->geterr() ? s->errstr() : "no error";
        log(WvLog::Error, "%s is closed (%s); restarting\n",
                id ? id : "Stream", err);
        restart();
    }
}

void WvStreamsDaemon::die_close_cb(IWvStream *s, const char *id)
{
    if (should_run())
    {
	WvString err = s->geterr() ? s->errstr() : "no error";
        log(WvLog::Error, "%s is closed (%s); dying\n",
	    id ? id : "Stream", err);
        die();
    }
}

void WvStreamsDaemon::setcallback(WvDaemonCallback cb)
{
    callback = cb;
}

