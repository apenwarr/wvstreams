/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "uniconfpamconn.h"
#include "wvunixsocket.h"
#include "wvtcp.h"
#include "wvsslstream.h"


UniConfDaemon::UniConfDaemon(const UniConf &_cfg, bool auth)
    : cfg(_cfg), log("UniConfDaemon"), debug(log.split(WvLog::Debug1)),
      closed(false), authenticate(auth)
{
    debug("Starting.\n");
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
        debug("Saving changes.\n");
        cfg.commit();
        debug("Done saving changes.\n");
    }
}


bool UniConfDaemon::isok() const
{
    return !closed && WvStreamList::isok();
}


void UniConfDaemon::accept(WvStream *stream)
{
    debug("Accepting connection from %s.\n", *stream->src());
    if (authenticate)
        append(new UniConfPamConn(stream, cfg), true);
    else
        append(new UniConfDaemonConn(stream, cfg), true);
}


void UniConfDaemon::unixcallback(WvStream &l, void *)
{
    debug("Incoming Unix domain connection.\n");
    WvUnixListener *listener = static_cast<WvUnixListener*>(& l);
    WvStream *s = listener->accept();
    accept(s);
}


void UniConfDaemon::tcpcallback(WvStream &l, void *)
{
    WvTCPListener *listener = static_cast<WvTCPListener*>(& l);
    WvStream *s = listener->accept();
    debug("Incoming TCP connection from %s.\n", *s->src());
    accept(s);
}


void UniConfDaemon::sslcallback(WvStream &l, void *userdata)
{
    WvX509Mgr *x509 = static_cast<WvX509Mgr *>(userdata);
    WvTCPListener *listener = static_cast<WvTCPListener *>(&l);
    WvStream *s = listener->accept();
    debug("Incoming TCP/SSL connection from %s.\n", *s->src());
    accept(new WvSSLStream(s, x509, false, true));
}


bool UniConfDaemon::setupunixsocket(WvStringParm path, int create_mode)
{
    WvUnixListener *listener = new WvUnixListener(path, create_mode);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create Unix domain socket: %s\n",
            listener->errstr());
        RELEASE(listener);
        return false;
    }
    listener->setcallback(WvStreamCallback(this,
        &UniConfDaemon::unixcallback), NULL);
    append(listener, true, "WvUnixListener");
    debug("Listening on Unix socket '%s'\n", path);
    return true;
}


bool UniConfDaemon::setuptcpsocket(const WvIPPortAddr &addr)
{
    WvTCPListener *listener = new WvTCPListener(addr);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create TCP socket: %s\n",
            listener->errstr());
        RELEASE(listener);
        return false;
    }
    listener->setcallback(WvStreamCallback(this,
        &UniConfDaemon::tcpcallback), NULL);
    append(listener, true, "WvTCPListener");
    debug("Listening for TCP at %s.\n", addr);
    return true;
}


bool UniConfDaemon::setupsslsocket(const WvIPPortAddr &addr, WvX509Mgr *x509)
{
    WvTCPListener *listener = new WvTCPListener(addr);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create SSL socket: %s\n",
            listener->errstr());
        RELEASE(listener);
        return false;
    }
    listener->setcallback(WvStreamCallback(this,
        &UniConfDaemon::sslcallback), x509);
    append(listener, true, "WvTCPListener(SSL)");
    debug("Listening for TCP/SSL at %s.\n", addr);
    return true;
}
