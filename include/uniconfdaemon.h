/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon.
 */
#ifndef __UNICONFDAEMON_H
#define __UNICONFDAEMON_H

#include "wvlog.h"
#include "wvistreamlist.h"
#include "uniconf.h"
#include "wvaddr.h"

class WvX509Mgr;

class UniConfDaemon : public WvIStreamList
{
    UniConf cfg;
    WvLog log, debug;
    bool authenticate;
    IUniConfGen *permgen;

public:
    /**
     * Create a UniConfDaemon to serve the Uniconf tree cfg.  If auth is
     * true, require authentication through PAM before accepting connections.
     */
    UniConfDaemon(const UniConf &cfg, bool auth, IUniConfGen *permgen);
    virtual ~UniConfDaemon();

    virtual void close();

    void accept(WvStream *stream);
    
    /**
     * Sets up a unix domain socket to allow for communication with the
     * daemon. Returns 'true' if successful, false otherwise.
     */
    bool setupunixsocket(WvStringParm path, int create_mode = 0755);
    /**
     * Sets up a tcp socket to allow for communication with the
     * daemon. Returns 'true' if successful, false otherwise.
     */
    bool setuptcpsocket(const WvIPPortAddr &addr);
    /**
     * Sets up an ssl-encrypted tcp socket to allow for communication with 
     * the daemon. Returns 'true' if successful, false otherwise.
     */
    bool setupsslsocket(const WvIPPortAddr &addr, WvX509Mgr *x509);

private:
    void unixcallback(WvStream &s, void *userdata);
    void tcpcallback(WvStream &s, void *userdata);
    void sslcallback(WvStream &s, void *userdata);
};

#endif // __UNICONFDAEMON_H
