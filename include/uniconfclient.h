/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConfClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#ifndef __UNICONFCLIENT_H
#define __UNICONFCLIENT_H

#include "uniconf.h"
#include "uniconfpair.h"
#include "uniconfiter.h"
#include "uniconfconn.h"
#include "wvlog.h"
#include "wvstream.h"
#include "wvstreamlist.h"


/**
 * Communicates with a UniConfDaemon to fetch and store keys and
 * values.
 * <p>
 * To mount, use the moniker prefix "unix://" followed by the
 * path of the Unix domain socket used by the UniConfDaemon.
 * Alternately, use the moniker prefix "tcp://" followed by the
 * hostname, a colon, and the port of a machine that serves
 * UniConfDaemon requests over TCP.
 * </p>
 */
class UniConfClientGen : public UniConfGen
{
public:
    UniConf *top;
    UniConfConn *conn;
    WvLog log;
    UniConfPairDict waiting;
    
    UniConfClientGen(const UniConfLocation &location,
        UniConf *_top, WvStream *stream, WvStreamList *l = NULL);
    ~UniConfClientGen();

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
    virtual void enumerate_subtrees(UniConf *conf, bool recursive);
    virtual void update(UniConf *&h);
    virtual void pre_get(UniConf *&h);
    // updates all data I am responsible for.
    virtual void update_all();
    virtual bool isok();
    virtual void save();

protected:
    void execute(WvStream &s, void *userdata);
    void executereturn(UniConfKey &key, WvConstStringBuffer &fromline);
    void executeforget(UniConfKey &key);
    void executesubtree(UniConfKey &key, WvConstStringBuffer &fromline);
    void executeok(WvConstStringBuffer &fromline);
    void executefail(WvConstStringBuffer &fromline);
    void savesubtree(UniConf *tree, UniConfKey key);
    bool waitforsubt;
private:
    WvStreamList *list;
};


/**
 * A factory for UniConfClientGen instances.
 */
class UniConfClientGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top);
};

#endif // __UNICONFCLIENT_H
