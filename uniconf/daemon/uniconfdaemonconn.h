/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session.
 */
#ifndef __UNICONFDAEMONCONN_H
#define __UNICONFDAEMONCONN_H

#include "uniconf.h"
#include "uniclientconn.h"
#include "unipermgen.h"
#include "wvlog.h"
#include "wvhashtable.h"

#define NUM_WATCHES 113

class UniConfDaemon;

/** Data structure to track requested watches */
struct UniConfDaemonWatch
{
    UniConfKey key;
    bool recurse;

    UniConfDaemonWatch(const UniConfKey &_key, bool _recurse)
        : key(_key), recurse(_recurse) { }

    bool operator== (const UniConfDaemonWatch &other) const
    {
        return key == other.key && recurse == other.recurse;
    }

    // annoying identity function
    static const UniConfDaemonWatch *get_key(const UniConfDaemonWatch *e)
        { return e; }
};
extern unsigned WvHash(const UniConfDaemonWatch &watch);

typedef WvHashTable<UniConfDaemonWatch, UniConfDaemonWatch,
    UniConfDaemonWatch> UniConfDaemonWatchTable;


/**
 * Retains all state and behavior related to a single UniConf daemon
 * connection.
 */
class UniConfDaemonConn : public UniClientConn 
{
public:
    UniConfDaemonConn(WvStream *s, const UniConf &root);
    virtual ~UniConfDaemonConn() { close(); }

    virtual void close();

    virtual void execute();

protected:
    UniConf root;
    UniConfDaemonWatchTable watches;

    virtual void do_malformed();
    virtual void do_noop();
    virtual void do_reply(WvStringParm reply);
    virtual void do_get(const UniConfKey &key);
    virtual void do_set(const UniConfKey &key, WvStringParm value);
    virtual void do_remove(const UniConfKey &key);
    virtual void do_subtree(const UniConfKey &key);
    virtual void do_haschildren(const UniConfKey &key);
    virtual void do_quit();
    virtual void do_help();

    virtual void addcallback();
    virtual void delcallback();

    void deltacallback(const UniConf &cfg, const UniConfKey &key);
};

#endif // __UNICONFDAEMONCONN_H
