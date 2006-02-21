/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */ 
#ifndef __WVUNIXSOCKET_H
#define __WVUNIXSOCKET_H

#include "wvfdstream.h"
#include "wvaddr.h"

class WvIStreamList;
class WvUnixListener;

/**
 * WvStream-based Unix domain socket connection class.
 * 
 * Unlike WvTCPConn, WvUnixConn makes connections synchronously because either
 * the remote server is there, or it isn't.  For convenience, we'll just
 * ignore situations where it's a local server but still slow.
 * 
 * FIXME: support SOCK_DGRAM mode somehow.  This is a bit tricky since the
 * listener/connection separation doesn't make as much sense then.  I guess we
 * could just ignore the listener or something...
 * 
 * FIXME: use the weird credential-passing stuff to exchange pid, uid, and gid
 * with the remote end of the socket.  See the unix(7) man page.  This would
 * be very cool for authentication purposes.
 */
class WvUnixConn : public WvFDStream
{
    friend class WvUnixListener;
protected:
    WvUnixAddr addr;
    
    /** connect an already-open socket (used by WvUnixListener) */
    WvUnixConn(int _fd, const WvUnixAddr &_addr);
    
public:
    /** connect a new socket */
    WvUnixConn(const WvUnixAddr &_addr);

    virtual ~WvUnixConn();
    
    /**
     * the local address of this socket (ie. from getsockname())
     * really useful only for transparent proxies, but always available.
     * may be 0.0.0.0 if we did not bind explicitly!
     */
    const WvUnixAddr &localaddr() { return addr; }
    
    /**
     * return the remote address (source of all incoming packets),
     * which is a constant for any given connection.
     * This doesn't make much sense in Unix domain sockets, so we just
     * return localaddr() instead.
     */
    virtual const WvUnixAddr *src() const;
    
public:
    const char *wstype() const { return "WvUnixConn"; }
};

/** Server end of a Unix Sockets stream */
class WvUnixListener : public WvFDStream
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
    WvUnixConn *accept();
    
    /**
     * set a callback() function that automatically accepts new WvUnixConn
     * connections, assigning them their own callback function 'callfunc'
     * with parameter 'userdata.'  Pass list==NULL or define your own
     * own callback function to disable auto-accepting.
     *
     * Be careful not to accept() connections yourself if you do this,
     * or we may end up accept()ing twice, causing a hang the second time.
     */
    void auto_accept(WvIStreamList *list,
		     WvStreamCallback callfunc = NULL, void *userdata = NULL);

    /**
     * these don't do anything, but they confuse the socket, so we'll
     * ignore them on purpose.
     */
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);
    
    /** src() is a bit of a misnomer, but it returns the socket address. */
    virtual const WvUnixAddr *src() const;
    
protected:
    WvUnixAddr addr;
    bool bound_okay;
    WvIStreamList *auto_list;

    WvStreamCallback auto_callback;
    void *auto_userdata;

    static void accept_callback(WvStream &s, void *userdata);

public:
    const char *wstype() const { return "WvUnixListener"; }
};


#endif // __WVUNIXSOCKET_H
