/* -*- Mode: C++ -*-
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
#include "uniclientconn.h"


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

    struct KeyVal
    {
	UniConfKey key;
	WvString val;
	
	KeyVal(const UniConfKey &_key, WvStringParm _val)
	    : key(_key), val(_val)
	    { }
    };
    DeclareWvList(KeyVal);

    WvLog log;

    WvString result_key;        /*!< the key that the current result is from */
    WvString result;            /*!< the result from the current key */
    
    KeyValList *result_list;    /*!< result list for iterations */

    bool cmdinprogress;     /*!< true while a command is in progress */
    bool cmdsuccess;        /*!< true when a command completed successfully */

    static const int TIMEOUT = 30000; // command timeout in ms

public:
    /**
     * Creates a generator which can communicate with a daemon using
     * the specified stream.
     * "stream" is the raw connection
     */
    UniClientGen(IWvStream *stream, WvStringParm dst = WvString::null);

    virtual ~UniClientGen();

    /***** Overridden members *****/

    virtual bool isok();

    virtual bool refresh();
    virtual void flush_buffers();
    virtual void commit(); 
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
    virtual Iter *recursiveiterator(const UniConfKey &key);

protected:
    virtual Iter *do_iterator(const UniConfKey &key, bool recursive);
    void conncallback(WvStream &s, void *userdata);
    bool do_select();
};


#endif // __UNICONFCLIENT_H
