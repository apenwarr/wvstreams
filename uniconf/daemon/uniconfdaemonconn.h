/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** /file
 * Manages a UniConf daemon session.
 */
#ifndef __UNICONFDAEMONCONN_H
#define __UNICONFDAEMONCONN_H

#include "uniconf.h"
#include "uniconfconn.h"
#include "wvlog.h"

class UniConfDaemon;

class UniConfDaemonConn : public UniConfConn 
{
    UniConf root;

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
};

#endif // __UNICONFDAEMONCONN_H
