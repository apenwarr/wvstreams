/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "uniconfiter.h"
#include "wvunixsocket.h"
#include "wvtcp.h"

#define debug(msg) \
    log(WvLog::Debug1, WvString("in %s: %s", __LINE__, msg))


UniConfDaemon::UniConfDaemon(const UniConf &_cfg) :
    cfg(_cfg), log("UniConfDaemon"), closed(false)
{
}

UniConfDaemon::~UniConfDaemon()
{
    close();
}


void UniConfDaemon::close()
{
    if (! closed)
    {
        closed = true;
        debug("Saving changes\n");
        cfg.commit();
        debug("Done saving changes\n");
    }
}


bool UniConfDaemon::isok() const
{
    return ! closed && WvStreamList::isok();
}


void UniConfDaemon::accept(WvStream *stream)
{
    debug(WvString("Accepting connection from: %s\n", *stream->src()));
    UniConfDaemonConn *conn = new UniConfDaemonConn(stream, cfg);
    append(conn, true);
}


void UniConfDaemon::unixcallback(WvStream &l, void *)
{
    debug("Incoming Unix domain connection\n");
    WvUnixListener *listener = static_cast<WvUnixListener*>(& l);
    WvStream *s = listener->accept();
    accept(s);
}


void UniConfDaemon::tcpcallback(WvStream &l, void *)
{
    debug("Incoming TCP connection\n");
    WvTCPListener *listener = static_cast<WvTCPListener*>(& l);
    WvStream *s = listener->accept();
    accept(s);
}


bool UniConfDaemon::setupunixsocket(WvStringParm path)
{
    WvUnixListener *listener = new WvUnixListener(path, 0755);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create Unix domain socket: %s\n",
            listener->errstr());
        delete listener;
        return false;
    }
    listener->setcallback(WvStreamCallback(this,
        &UniConfDaemon::unixcallback), NULL);
    append(listener, true, "WvUnixListener");
    return true;
}


bool UniConfDaemon::setuptcpsocket(const WvIPPortAddr &addr)
{
    WvTCPListener *listener = new WvTCPListener(addr);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create TCP socket: %s\n",
            listener->errstr());
        delete listener;
        return false;
    }
    listener->setcallback(WvStreamCallback(this,
        &UniConfDaemon::tcpcallback), NULL);
    append(listener, true, "WvTCPListener");
    return true;
}
