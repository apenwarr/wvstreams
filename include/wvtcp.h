/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStream-based TCP connection and server classes.
 */ 
#ifndef __WVTCP_H
#define __WVTCP_H

#include "wvstream.h"
#include "wvaddr.h"
#include "wvresolver.h"


#if 0
// FIXME: this is needed on BSD
#include <netinet/in_systm.h>
#define SOL_TCP 6
#define SOL_IP 4
#endif

#ifdef ISBSD
#define socklen_t unsigned int
#endif

#ifdef ISDARWIN
#define socklen_t int
#endif


class WvStreamList;
class WvTCPListener;

/**
 * WvTCPConn tries to make all outgoing connections asynchronously (in
 * the background).  You can tell the connection has been established
 * when a select() call returns 'true' with writable==true.
 */
class WvTCPConn : public WvStream
{
    friend class WvTCPListener;
protected:
    bool resolved, connected;
    WvString hostname;
    WvIPPortAddr remaddr;
    WvResolver dns;
    
    /**
     * Start a WvTCPConn on an already-open socket (used by WvTCPListener)
     */
    WvTCPConn(int _fd, const WvIPPortAddr &_remaddr);
    
    /**
     * Connect to the remote end - note the "Protected" above ;)
     */
    void do_connect();
    
    /**
     * Resolve the remote address, if it was fed in non-IP form
     */
    void check_resolver();
    
public:
   /**
    * WvTCPConn tries to make all outgoing connections asynchronously (in
    * the background).  You can tell the connection has been established
    * when a select() call returns 'true' with writable==true.
    */
    WvTCPConn(const WvIPPortAddr &_remaddr);
    
    /**
     * Resolve the hostname, then connect a new socket
     */
    WvTCPConn(WvStringParm _hostname, __u16 _port = 0);

    /**
     * Destructor - rarely do you need to call this - close()
     * is a much better way to tear down a TCP Stream ;)
     */
    virtual ~WvTCPConn();
    
    /**
     * function to set up a TCP socket the way we like
     * (Read/Write, Non-Blocking, KeepAlive)
     */
    void nice_tcpopts();
    
    /**
     * function to set up a TCP socket the way we like
     * In addition to the nice_tcpopts(), set TCP_NODELAY
     */
    void low_delay();
    
    /**
     * the local address of this socket (ie. from getsockname())
     * really useful only for transparent proxies, but always available.
     * may be 0.0.0.0 if we did not bind explicitly!
     */
    WvIPPortAddr localaddr();
    
    /**
     * return the remote address (source of all incoming packets),
     * which is a constant for any given TCP connection.
     */
    virtual const WvIPPortAddr *src() const;

    /**
     * has the connection been completed yet?
     */
    bool isconnected() const
        { return connected; }
    
    /**
     * override pre_select() to cause select() results when resolving names.
     */
    virtual bool pre_select(SelectInfo &si);
    
    /**
     * override post_select() to set the 'connected' variable as soon as we
     * are connected.
     */
    virtual bool post_select(SelectInfo &si);
    
    /**
     * Is this connection OK? 
     * Note: isok() will always be true if !resolved, even though fd==-1.
     */
    virtual bool isok() const;
};

/**
 * Class to easily create the Server side of a TCPConn...
 */
class WvTCPListener : public WvStream
{
public:
    /**
     * Create a WvStream that listens on _listenport of the current machine
     * This is how you set up a TCP Server.
     */
    WvTCPListener(const WvIPPortAddr &_listenport);

    /**
     * Destructor - remember - close() is your friend ;)
     */
    virtual ~WvTCPListener();
    
    /**
     * Shut down the server, and disconnect from the port
     */
    virtual void close();
    
    /**
     * return a new WvTCPConn socket corresponding to a newly-accepted
     * connection.  If no connection is ready immediately, we wait for
     * one indefinitely.  You can use select(read=true) to check for a
     * waiting connection.
     */
    WvTCPConn *accept();
    
    /**
     * set a callback() function that automatically accepts new WvTCPConn
     * connections, assigning them their own callback function 'callfunc'
     * with parameter 'userdata.'  Pass list==NULL or define your own
     * own callback function to disable auto-accepting.
     *
     * Be careful not to accept() connections yourself if you do this,
     * or we may end up accept()ing twice, causing a hang the second time.
     */
    void auto_accept(WvStreamList *list,
		     WvStreamCallback callfunc = NULL, void *userdata = NULL);

    /**
     * these don't do anything, but they confuse the socket, so we'll
     * ignore them on purpose.
     */
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);
    
    /**
     * src() is a bit of a misnomer, but it returns the listener port.
     */
    virtual const WvIPPortAddr *src() const;
    
protected:
    WvIPPortAddr listenport;
    WvStreamList *auto_list;
    WvStreamCallback auto_callback;
    void *auto_userdata;
    
    static void accept_callback(WvStream &s, void *userdata);
};


#endif // __WVTCP_H
