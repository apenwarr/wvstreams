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
    void do_subtree(const UniConfKey &key);
    void do_haschildren(const UniConfKey &key);
    void do_addwatch(const UniConfKey &key, bool recurse = true);
    void do_delwatch(const UniConfKey &key, bool recurse = true);
    void do_quit();
    void do_help();

    void deltacallback(const UniConf &key, void *userdata);
};

#endif // __UNICONFDAEMONCONN_H
