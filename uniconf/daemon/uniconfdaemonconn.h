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
#include "wvlog.h"
#include "wvhashtable.h"

class UniConfDaemon;

/** Data structure to track requested watches */
struct UniConfDaemonWatch
{
    UniConfKey key;
    UniConfDepth::Type depth;

    UniConfDaemonWatch(const UniConfKey &_key, UniConfDepth::Type _depth)
        : key(_key), depth(_depth) { }

    bool operator== (const UniConfDaemonWatch &other) const
    {
        return key == other.key && depth == other.depth;
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
    UniConf root;
    UniConfDaemonWatchTable watches;

public:
    UniConfDaemonConn(WvStream *s, const UniConf &root);
    virtual ~UniConfDaemonConn();

    virtual void execute();

private:
    void do_malformed();
    void do_noop();
    void do_get(const UniConfKey &key);
    void do_set(const UniConfKey &key, WvStringParm value);
    void do_remove(const UniConfKey &key);
    void do_zap(const UniConfKey &key);
    void do_subtree(const UniConfKey &key);
    void do_addwatch(const UniConfKey &key, UniConfDepth::Type depth);
    void do_delwatch(const UniConfKey &key, UniConfDepth::Type depth);
    void do_quit();
    void do_help();

    void deltacallback(const UniConf &key, void *userdata);
};

#endif // __UNICONFDAEMONCONN_H
