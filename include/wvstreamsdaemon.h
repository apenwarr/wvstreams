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

    WvIStreamList streams;

    void start_cb(WvDaemon &daemon, void *);
    void run_cb(WvDaemon &daemon, void *);
    void stop_cb(WvDaemon &daemon, void *);
    void stop_full_close_cb(WvDaemon &daemon, void *);

    void restart_close_cb(const char *, WvStream &);
    void die_close_cb(const char *, WvStream &);

public:

    WvStreamsDaemon(WvStringParm name, WvStringParm version,
		    WvStreamsDaemonCallback cb, void *ud = NULL);

    void add_stream(IWvStream *istream,
		    bool auto_free, const char *id = NULL);
    void add_restart_stream(IWvStream *istream,
			    bool auto_free, const char *id = NULL);
    void add_die_stream(IWvStream *istream,
			bool auto_free, const char *id = NULL);

    void close_existing_connections_on_restart();
};

#endif // __WVSTREAMSDAEMON_H
