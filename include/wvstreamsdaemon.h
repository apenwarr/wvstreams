/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * High-level abstraction for creating daemon processes that do
 * nothing but listen on a list of WvStreams and add connections
 * to the global list.
 */
#ifndef __WVSTREAMSDAEMON_H
#define __WVSTREAMSDAEMON_H

#include "wvdaemon.h"
#include "iwvstream.h"
#include "wvistreamlist.h"

class WvStreamsDaemon;

typedef WvCallback<void, WvStreamsDaemon &, void *> WvStreamsDaemonCallback;

class WvStreamsDaemon : public WvDaemon
{
    private:

        WvStreamsDaemonCallback callback;
        void *userdata;

        WvIStreamList listeners;

        void start_cb(WvDaemon &daemon, void *);
        void run_cb(WvDaemon &daemon, void *);
        void stop_cb(WvDaemon &daemon, void *);
        void stop_full_close_cb(WvDaemon &daemon, void *);

        void listener_close_cb(WvStream &s, void *);

    public:

        WvStreamsDaemon(WvStringParm name, WvStringParm version,
                WvStreamsDaemonCallback cb, void *ud = NULL);

        void add_listener(IWvStream *istream);

        void close_existing_connections_on_restart();
};

#endif // __WVSTREAMSDAEMON_H
