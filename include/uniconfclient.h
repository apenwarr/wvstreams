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

#include "uniconfgen.h"
#include "uniconfconn.h"
#include "uniconfpair.h"
#include "wvlog.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvstringlist.h"


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
    UniConfLocation xlocation;
    UniConfConn *conn;
    WvLog log;
    UniConfPairDict waiting;

    bool inprogress; /*!< true while a command is in progress */
    bool success; /*!< true when a command completed successfully */
    bool justneedok; /*!< true when a command just needs okay
        rather than an extended response */

public:
    UniConfClientGen(const UniConfLocation &location,
        WvStream *stream);
    virtual ~UniConfClientGen();

    /***** Overridden members *****/

    virtual UniConfLocation location() const;
    virtual bool isok();
    virtual void attach(WvStreamList *streamlist);
    virtual void detach(WvStreamList *streamlist);

    virtual bool refresh(const UniConfKey &key, UniConf::Depth depth);
    virtual bool commit(const UniConfKey &key, UniConf::Depth depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);

    class RemoteKeyIter;

protected:
    void execute(WvStream &s, void *userdata);
    void executereturn(UniConfKey &key, WvBuffer &fromline);
    void executeforget(UniConfKey &key);
    void executesubtree(UniConfKey &key, WvBuffer &fromline);
    void executeok(WvBuffer &fromline);
    void executefail(WvBuffer &fromline);
    
    bool wait(bool justneedok = false);
};


/**
 * An iterator over remote keys.
 */
class UniConfClientGen::RemoteKeyIter : public UniConfClientGen::Iter
{
protected:
    WvStringList *xlist;
    WvStringList::Iter xit;

public:
    RemoteKeyIter(WvStringList *list);
    virtual ~RemoteKeyIter();

    /***** Overridden methods *****/

    virtual RemoteKeyIter *clone() const;
    virtual void rewind();
    virtual bool next();
    virtual UniConfKey key() const;
};


/**
 * A factory for UniConfClientGen instances.
 */
class UniConfClientGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfGen *newgen(const UniConfLocation &location);
};

#endif // __UNICONFCLIENT_H
