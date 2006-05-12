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

/*!
@brief WvStreamsDaemon - High-level abstraction for a daemon process that
does nothing but add streams to the global list and execute it.

This is generally what a modern WvStreams-based daemon should look like.

The WvStreamsDaemonCallback function passed in the constructor is used to 
populate the globallist with streams that are necessary when the daemon
starts, such as listening sockets.  These streams are added using the
WvStreamsDaemon::add_stream, WvStreamsDaemon::add_die_stream and
WvStreamsDaemon::add_restart_stream members, the last two governing what
happens to the daemon when the stream is !isok().

Sample usage:

@code
#include <wvstreams/wvdaemon.h>

void client_cb(WvStream &stream, void *)
{
    // Echo everything back
    const char *line = stream.getline();
    if (line)
        stream.print("You said: %s\n", line);
}     

void startup(WvStreamsDaemon &daemon, void *)
{
    WvUnixListener *listener = new WvUnixListener("/tmp/socket", 0700);
    listener->auto_accept(&WvIStreamList::globallist, client_cb); 
    daemon.add_die_stream(listener, true, "Listener");
}

int main(int argc, char **argv)
{
    WvStreamsDaemon daemon("Sample Daemon", "0.1", startup);

    return daemon.run(argc, argv);
}
@endcode
!*/
class WvStreamsDaemon : public WvDaemon
{
private:

    WvStreamsDaemonCallback callback;
    void *userdata;

    bool do_full_close;
    WvIStreamList streams;

    void init(WvStreamsDaemonCallback cb, void *ud);

protected:

    virtual void do_start();
    virtual void do_run();
    virtual void do_stop();

private:

    void restart_close_cb(const char *, WvStream &);
    void die_close_cb(const char *, WvStream &);

public:

    //! Construct a new WvStreamsDaemon with given name and version, and
    //! use the cb function to populate the daemon with its initial streams
    WvStreamsDaemon(WvStringParm name,
            WvStringParm version,
            WvStreamsDaemonCallback cb,
            void *ud = NULL) :
        WvDaemon(name, version, WvDaemonCallback(),
                WvDaemonCallback(), WvDaemonCallback())
    {
        init(cb, ud);
    }

    //! Construct a new WvStreamsDaemon with given name and
    //! use the cb function to populate the daemon with its initial streams
    WvStreamsDaemon(WvStringParm name, 
            WvStreamsDaemonCallback cb,
            void *ud = NULL) :
        WvDaemon(name, WvDaemonCallback(),
                WvDaemonCallback(), WvDaemonCallback())
    {
        init(cb, ud);
    }

    //! Add a stream to the daemon; don't do anything if it goes !isok().
    //! This should be called from the WvStreamsDaemonCallback function
    //! passed to the constructor.
    void add_stream(IWvStream *istream,
		    bool auto_free, const char *id = NULL);
    //! Add a stream to the daemon; the daemon will restart, re-populating
    //! the initial streams using the callback passed to the constructor,
    //! if the stream goes !isok().
    //! This should be called from the WvStreamsDaemonCallback function
    //! passed to the constructor.
    void add_restart_stream(IWvStream *istream,
			    bool auto_free, const char *id = NULL);
    //! Add a stream to the daemon; if the stream goes !isok() the daemon
    //! will exit.
    //! This should be called from the WvStreamsDaemonCallback function
    //! passed to the constructor.
    void add_die_stream(IWvStream *istream,
			bool auto_free, const char *id = NULL);

    //! If this member is called then any existing streams on the globallist 
    //! added *after* the WvStreamsDaemonCallback was executed will be closed
    //! if the daemon restarts; otherwise, they will persist after the restart.
    void close_existing_connections_on_restart()
    {
        do_full_close = true;
    }

    //! Change the callback function and userdata
    void setcallback(WvStreamsDaemonCallback cb, void *ud = NULL);

public:
    const char *wstype() const { return "WvStreamsDaemon"; }
};
#endif // __WVSTREAMSDAEMON_H
