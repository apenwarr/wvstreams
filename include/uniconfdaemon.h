/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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
    bool closed;
    bool authenticate;

public:
    /**
     * Create a UniConfDaemon to serve the Uniconf tree cfg.  If auth is
     * true, require authentication through PAM before accepting connections.
     */
    UniConfDaemon(const UniConf &cfg, bool auth);
    virtual ~UniConfDaemon();

    virtual bool isok() const;
    virtual void close();

    void accept(WvStream *stream);
    
    bool setupunixsocket(WvStringParm path, int create_mode = 0755);
    bool setuptcpsocket(const WvIPPortAddr &addr);
    bool setupsslsocket(const WvIPPortAddr &addr, WvX509Mgr *x509);

private:
    void unixcallback(WvStream &s, void *userdata);
    void tcpcallback(WvStream &s, void *userdata);
    void sslcallback(WvStream &s, void *userdata);
};

#endif // __UNICONFDAEMON_H
