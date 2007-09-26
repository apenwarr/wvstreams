/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2004 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"

#ifndef _WIN32
#include "uniconfpamconn.h"
#include "wvunixsocket.h"
#endif

#include "wvtcp.h"
#include "wvsslstream.h"
#include "uninullgen.h"


UniConfDaemon::UniConfDaemon(const UniConf &_cfg,
			     bool auth, IUniConfGen *_permgen)
    : cfg(_cfg), log("UniConf Daemon"), debug(log.split(WvLog::Debug1))
{
    authenticate = auth;

#ifdef _WIN32
    assert(!authenticate);
#endif

    permgen = _permgen ? _permgen : new UniNullGen();
    debug("Starting.\n");
}


UniConfDaemon::~UniConfDaemon()
{
    close();
    WVRELEASE(permgen);
}


void UniConfDaemon::close()
{
    if (!closed)
    {
        debug("Saving changes.\n");
        cfg.commit();
        debug("Done saving changes.\n");
    }
    
    WvIStreamList::close();
}


void UniConfDaemon::accept(WvStream *stream)
{
    debug("Accepting connection from %s.\n", *stream->src());
    
    // FIXME: permgen should be used regardless of whether we authenticate,
    // and there should be a command to authenticate explicitly.  That way we
    // can support access control for anonymous connections.
#ifndef _WIN32
    if (authenticate)
        append(new UniConfPamConn(stream, cfg,
				  new UniPermGen(permgen)), true, "ucpamconn");
    else
#endif
        append(new UniConfDaemonConn(stream, cfg), true, "ucdaemonconn");
}


#ifndef _WIN32
void UniConfDaemon::unixcallback(WvStream &l, void *)
{
    debug("Incoming Unix domain connection.\n");
    WvUnixListener *listener = static_cast<WvUnixListener*>(& l);
    WvStream *s = listener->accept();
    accept(s);
}
#endif


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
    accept(new WvSSLStream(s, x509, 0, true));
}


#ifndef _WIN32
bool UniConfDaemon::setupunixsocket(WvStringParm path, int create_mode)
{
    WvUnixListener *listener = new WvUnixListener(path, create_mode);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create Unix domain socket: %s\n",
            listener->errstr());
        WVRELEASE(listener);
        return false;
    }
    listener->setcallback(wv::bind(&UniConfDaemon::unixcallback, this, wv::_1,
				   wv::_2), NULL);
    append(listener, true, "unix listen");
    debug("Listening on Unix socket '%s'\n", path);
    return true;
}
#endif


bool UniConfDaemon::setuptcpsocket(const WvIPPortAddr &addr)
{
    WvTCPListener *listener = new WvTCPListener(addr);
    if (! listener->isok())
    {
        log(WvLog::Error, "Could not create TCP socket: %s\n",
            listener->errstr());
        WVRELEASE(listener);
        return false;
    }
    listener->setcallback(wv::bind(&UniConfDaemon::tcpcallback, this, wv::_1,
				   wv::_2), NULL);
    append(listener, true, "tcp listen");
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
        WVRELEASE(listener);
        return false;
    }
    listener->setcallback(wv::bind(&UniConfDaemon::sslcallback, this, wv::_1,
				   wv::_2), x509);
    append(listener, true, "ssl listen");
    debug("Listening for TCP/SSL at %s.\n", addr);
    return true;
}
