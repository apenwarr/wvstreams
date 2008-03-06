/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2004 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "wvlistener.h"
#include "uninullgen.h"

#ifndef _WIN32
#include "uniconfpamconn.h"
#endif


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


void UniConfDaemon::listencallback(IWvStream *s)
{
    const WvAddr *a = s->src();
    if (a)
	debug("Incoming connection from %s.\n", *a);
    else
	debug("Incoming connection from UNKNOWN.\n");
    if (s->geterr())
    {
	debug("Error: %s\n", s->errstr());
	WVRELEASE(s);
    }
    else
	accept(new WvStreamClone(s));
}


void UniConfDaemon::listen(WvStringParm lmoniker)
{
    IWvListener *l = IWvListener::create(lmoniker);
    debug("Listening on %s.\n", *l->src());
    if (!l->isok())
    {
	log(WvLog::Error, "Can't listen: %s\n", l->errstr());
	seterr_both(l->geterr(), l->errstr());
	WVRELEASE(l);
    }
    else
    {
	l->onaccept(wv::bind(&UniConfDaemon::listencallback, this, _1));
	append(l, true, "listener");
    }
}
