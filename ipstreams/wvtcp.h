/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvStream-based TCP connection class.
 * 
 * WvTCPConn tries to make all outgoing connections asynchronously (in
 * the background).  You can tell the connection has been established
 * when a select() call returns 'true' with writable==true.
 */
#ifndef __WVTCP_H
#define __WVTCP_H

#include "wvstream.h"
#include "wvaddr.h"

class WvTCPListener;

class WvTCPConn : public WvStream
{
    friend class WvTCPListener;
protected:
    bool connected;
    WvIPPortAddr remaddr;
    
    // connect an already-open socket (used by WvTCPListener)
    WvTCPConn(int _fd, const WvIPPortAddr &_remaddr);
    
public:
    // connect a new socket
    WvTCPConn(const WvIPPortAddr &_remaddr);
    
    // return the remote address (source of all incoming packets),
    // which is always the same for TCP.
    virtual const WvAddr *src() const;

    // has the connection been completed yet?
    bool isconnected() const
        { return connected; }
    
    // override test_set() to set the 'connected' variable as soon as we
    // are connected.
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);
};


class WvTCPListener : public WvStream
{
public:
    WvTCPListener(const WvIPPortAddr &_listenport);
    
    // return a new WvTCPConn socket corresponding to a newly-accepted
    // connection.  If no connection is ready immediately, we wait for
    // one indefinitely.  You can use select(read=true) to check for a
    // waiting connection.
    WvTCPConn *accept();
    
    // set a callback() function that automatically accepts new WvTCPConn
    // connections, assigning them their own callback function 'callfunc'
    // with parameter 'userdata.'  Pass list==NULL or define your own
    // own callback function to disable auto-accepting.
    //
    // Be careful not to accept() connections yourself if you do this,
    // or we may end up accept()ing twice, causing a hang the second time.
    void auto_accept(WvStreamList *list,
		     Callback *callfunc = NULL, void *userdata = NULL);

    // these don't do anything, but they confuse the socket, so we'll
    // ignore them on purpose.
    virtual size_t uread(void *buf, size_t len);
    virtual size_t uwrite(const void *buf, size_t len);
    
    // src() is a bit of a misnomer, but it returns the listener port.
    virtual const WvAddr *src() const;

protected:
    WvIPPortAddr listenport;
    WvStreamList *auto_list;
    static Callback accept_callback;
    Callback *auto_callback;
    void *auto_userdata;
};


#endif // __WVTCP_H
