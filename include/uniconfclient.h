/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#ifndef __UNICONFCLIENT_H
#define __UNICONFCLIENT_H

#include "uniconfgen.h"
#include "wvlog.h"
#include "wvstringlist.h"

class UniConfConn;
class UniConfCache;

/**
 * Communicates with a UniConfDaemon to fetch and store keys and
 * values.
 * 
 * To mount, use the moniker prefix "unix:" followed by the
 * path of the Unix domain socket used by the UniConfDaemon.
 * Alternately, use the moniker prefix "tcp:" followed by the
 * hostname, a colon, and the port of a machine that serves
 * UniConfDaemon requests over TCP.
 * 
 */
class UniConfClientGen : public UniConfGen
{
    UniConfConn *conn;
    UniConfCache *cache;
    WvString streamid;
    WvLog log;

    bool cmdinprogress; /*!< true while a command is in progress */
    bool cmdsuccess; /*!< true when a command completed successfully */

    static const int TIMEOUT = 2000; // 2 sec timeout

public:
    /**
     * Creates a generator which can communicate with a daemon using
     * the specified stream.
     * @param stream the raw connection
     */
    UniConfClientGen(IWvStream *stream);

    virtual ~UniConfClientGen();

    /***** Overridden members *****/

    virtual bool isok();

    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);
    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);

    class RemoteKeyIter;

protected:
    void conncallback(WvStream &s, void *userdata);
    void prepare();
    bool wait();
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

#endif // __UNICONFCLIENT_H
