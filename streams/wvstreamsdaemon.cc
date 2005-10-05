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

WvStreamsDaemon::WvStreamsDaemon(WvStringParm name, WvStringParm version,
        WvStreamsDaemonCallback cb, void *ud)
    : WvDaemon(name, version,
                WvDaemonCallback(this, &WvStreamsDaemon::start_cb),
                WvDaemonCallback(this, &WvStreamsDaemon::run_cb),
                WvDaemonCallback(this, &WvStreamsDaemon::stop_cb))
{
    setcallback(cb, ud);
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
}

void WvStreamsDaemon::start_cb(WvDaemon &daemon, void *)
{
    callback(*this, userdata);
}

void WvStreamsDaemon::run_cb(WvDaemon &daemon, void *)
{
    if (streams.isempty())
    {
        log(WvLog::Error, "No streams; exiting\n");
        die();
    }

    while (daemon.should_run())
        WvIStreamList::globallist.runonce();
}

void WvStreamsDaemon::stop_cb(WvDaemon &daemon, void *)
{
    WvIStreamList::Iter stream(streams);
    for (stream.rewind(); stream.next(); )
        WvIStreamList::globallist.unlink(stream.ptr());
    streams.zap();
    if (want_to_die())
        WvIStreamList::globallist.zap();
}

void WvStreamsDaemon::stop_full_close_cb(WvDaemon &daemon, void *ud)
{
    stop_cb(daemon, ud);
    WvIStreamList::globallist.zap();
}

void WvStreamsDaemon::add_stream(IWvStream *istream,
    	bool autofree, const char *id)
{
    streams.append(istream, false, "WvStreamsDaemon stream");
    // FIXME: we should pass in "id" here, but things are not happy in
    // const-correctness-land.
    WvIStreamList::globallist.append(istream, autofree, "WvStreamsDaemon stream");
}

void WvStreamsDaemon::add_restart_stream(IWvStream *istream,
    	bool autofree, const char *id)
{
    add_stream(istream, autofree, id);
    
    istream->setclosecallback(
	WvBoundCallback<IWvStreamCallback, const char*>(this, &WvStreamsDaemon::restart_close_cb, id));
}

void WvStreamsDaemon::add_die_stream(IWvStream *istream,
    	bool autofree, const char *id)
{
    add_stream(istream, autofree, id);
    
    istream->setclosecallback(
	WvBoundCallback<IWvStreamCallback, const char*>(this, &WvStreamsDaemon::die_close_cb, id));
}

void WvStreamsDaemon::close_existing_connections_on_restart()
{
    stop_callback =
        WvDaemonCallback(this, &WvStreamsDaemon::stop_full_close_cb);
}

void WvStreamsDaemon::restart_close_cb(const char *id, WvStream &)
{
    if (should_run())
    {
        log(WvLog::Error, "%s is stale; restarting\n",
                id ? id : "Stream");
        restart();
    }
}

void WvStreamsDaemon::die_close_cb(const char *id, WvStream &)
{
    if (should_run())
    {
        log(WvLog::Error, "%s is stale; dying\n",
                id ? id : "Stream");
        die();
    }
}

void WvStreamsDaemon::setcallback(WvStreamsDaemonCallback cb, void *ud)
{
    callback = cb;
    userdata = ud;
}

