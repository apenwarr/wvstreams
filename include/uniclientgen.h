/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#ifndef __UNICONFCLIENT_H
#define __UNICONFCLIENT_H

#include "uniconfgen.h"
#include "wvlog.h"
#include "wvstringlist.h"

class UniClientConn;

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
class UniClientGen : public UniConfGen
{
    class RemoteKeyIter;

    UniClientConn *conn;
    WvStringList set_queue;
    WvLog log;

    WvString result_key;        /*!< the key that the current result is from */
    WvString result;            /*!< the result from the current key */
    WvStringList *result_list;  /*!< result list for iterations */

    bool cmdinprogress;     /*!< true while a command is in progress */
    bool cmdsuccess;        /*!< true when a command completed successfully */

    static const int TIMEOUT = 1000; // 1 sec timeout

public:
    /**
     * Creates a generator which can communicate with a daemon using
     * the specified stream.
     * "stream" is the raw connection
     */
    UniClientGen(IWvStream *stream);

    virtual ~UniClientGen();

    /***** Overridden members *****/

    virtual bool isok();

    virtual bool refresh();
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);

protected:
    void conncallback(WvStream &s, void *userdata);
    bool do_select();
};


/** An iterator over remote keys. */
class UniClientGen::RemoteKeyIter : public UniClientGen::Iter
{
protected:
    WvStringList *list;
    WvStringList::Iter i;

public:
    RemoteKeyIter(WvStringList *_list) : list(_list), i(*_list) { }
    virtual ~RemoteKeyIter() { delete list; }

    /***** Overridden methods *****/

    virtual RemoteKeyIter *clone() const;
    virtual void rewind();
    virtual bool next();
    virtual UniConfKey key() const;
};

#endif // __UNICONFCLIENT_H
