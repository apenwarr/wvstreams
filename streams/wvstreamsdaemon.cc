/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes that do
 * nothing but listen on a list of WvStreams and add connections
 * to the global list.
 */

#include "wvstreamsdaemon.h"

#include <signal.h>

WvStreamsDaemon::WvStreamsDaemon(WvStringParm name, WvStringParm version,
        WvStreamsDaemonCallback cb, void *ud)
    : WvDaemon(name, version,
                WvDaemonCallback(this, &WvStreamsDaemon::start_cb),
                WvDaemonCallback(this, &WvStreamsDaemon::run_cb),
                WvDaemonCallback(this, &WvStreamsDaemon::stop_cb)),
                callback(cb), userdata(ud)
{
    signal(SIGPIPE, SIG_IGN);
}

void WvStreamsDaemon::start_cb(WvDaemon &daemon, void *)
{
    callback(*this, userdata);
}

void WvStreamsDaemon::run_cb(WvDaemon &daemon, void *)
{
    if (streams.count() == 0)
    {
        log(WvLog::Error, "No streams; exiting\n");
        die();
    }

    while (daemon.should_run())
        WvIStreamList::globallist.runonce();
}

void WvStreamsDaemon::stop_cb(WvDaemon &daemon, void *)
{
    streams.zap();
}

void WvStreamsDaemon::stop_full_close_cb(WvDaemon &daemon, void *)
{
    WvIStreamList::globallist.zap();
    streams.zap();
}

void WvStreamsDaemon::add_stream(IWvStream *istream, const char *id)
{
    streams.append(istream, true);
    WvIStreamList::globallist.append(istream, false);
}

void WvStreamsDaemon::add_restart_stream(IWvStream *istream, const char *id)
{
    add_stream(istream);
    
    istream->setclosecallback(
            WvStreamCallback(this, &WvStreamsDaemon::restart_close_cb),
                    (void *)id);
}

void WvStreamsDaemon::add_die_stream(IWvStream *istream, const char *id)
{
    add_stream(istream);
    
    istream->setclosecallback(
            WvStreamCallback(this, &WvStreamsDaemon::die_close_cb), 
                    (void *)id);
}

void WvStreamsDaemon::close_existing_connections_on_restart()
{
    stop_callback =
        WvDaemonCallback(this, &WvStreamsDaemon::stop_full_close_cb);
}

void WvStreamsDaemon::restart_close_cb(WvStream &, void *ud)
{
    if (should_run())
    {
        const char *id = (const char *)ud;
        log(WvLog::Error, "%s is stale; restarting\n",
                id? id: "Stream");
        restart();
    }
}

void WvStreamsDaemon::die_close_cb(WvStream &, void *ud)
{
    if (should_run())
    {
        const char *id = (const char *)ud;
        log(WvLog::Error, "%s is stale; dying\n",
                id? id: "Stream");
        restart();
    }
}

