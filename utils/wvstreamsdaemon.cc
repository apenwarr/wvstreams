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
    if (listeners.count() == 0)
    {
        log(WvLog::Error, "No listeners; exiting\n");
        die();
    }

    while (daemon.should_run())
        WvIStreamList::globallist.runonce();
}

void WvStreamsDaemon::stop_cb(WvDaemon &daemon, void *)
{
    listeners.zap();
}

void WvStreamsDaemon::stop_full_close_cb(WvDaemon &daemon, void *)
{
    WvIStreamList::globallist.zap();
}

void WvStreamsDaemon::add_listener(IWvStream *istream)
{
    istream->setclosecallback(
            WvStreamCallback(this, &WvStreamsDaemon::listener_close_cb), NULL);

    listeners.append(istream, true);
    WvIStreamList::globallist.append(istream, false);
}

void WvStreamsDaemon::close_existing_connections_on_restart()
{
    stop_callback =
        WvDaemonCallback(this, &WvStreamsDaemon::stop_full_close_cb);
}

void WvStreamsDaemon::listener_close_cb(WvStream &, void *)
{
    if (should_run())
    {
        log(WvLog::Error, "Listener is stale; restarting\n");
        restart();
    }
}

