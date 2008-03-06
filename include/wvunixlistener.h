/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */ 
#ifndef __WVUNIXLISTENER_H
#define __WVUNIXLISTENER_H

#include "wvlistener.h"
#include "wvaddr.h"

#ifndef _WIN32

/** Server end of a Unix Sockets stream */
class WvUnixListener : public WvListener
{
public:
    WvUnixListener(const WvUnixAddr &_addr, int create_mode);
    virtual ~WvUnixListener();
    virtual void close();
    
    /**
     * return a new WvUnixConn socket corresponding to a newly-accepted
     * connection.  If no connection is ready immediately, we wait for
     * one indefinitely.  You can use select(read=true) to check for a
     * waiting connection.
     */
    IWvStream *accept();
    
    /**
     * Tell this listener to automatically accept new
     * connections, assigning them their own callback function 'cb'
     * Pass list==NULL or run setcallback() to disable auto-accepting.
     *
     * Be careful not to accept() connections yourself if you do this,
     * or we may end up accept()ing twice, causing a hang the second time.
     */
    void auto_accept(WvIStreamList *list, wv::function<void(IWvStream*)> cb);

    /**
     * Like auto_accept() above, but always uses the globallist instead
     * of a user-defined list.
     */
    void auto_accept(wv::function<void(IWvStream*)> cb);

    /** src() is a bit of a misnomer, but it returns the socket address. */
    virtual const WvUnixAddr *src() const;
    
protected:
    WvUnixAddr addr;
    bool bound_okay;

    void accept_callback(WvIStreamList *list,
			 wv::function<void(IWvStream*)> cb,
			 IWvStream *_connection);

public:
    const char *wstype() const { return "WvUnixListener"; }
};

#endif // _WIN32

#endif // __WVUNIXLISTENER_H
