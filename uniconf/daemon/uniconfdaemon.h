/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** /file
 * Manages a UniConf daemon.
 */
#ifndef __UNICONFDAEMON_H
#define __UNICONFDAEMON_H

#include "wvlog.h"
#include "wvstreamlist.h"
#include "uniconf.h"
#include "wvaddr.h"

class UniConfDaemon : public WvStreamList
{
    UniConf cfg;
    WvLog log;
    bool closed;

public:
    UniConfDaemon(const UniConf &cfg);
    virtual ~UniConfDaemon();

    virtual bool isok() const;
    virtual void close();

    void accept(WvStream *stream);
    
    bool setupunixsocket(WvStringParm path);
    bool setuptcpsocket(const WvIPPortAddr &addr);

private:
    void unixcallback(WvStream &s, void *userdata);
    void tcpcallback(WvStream &s, void *userdata);
};

#endif // __UNICONFDAEMON_H
