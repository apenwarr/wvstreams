/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session.
 */
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "wvtclstring.h"

/***** UniConfDaemonWatch ****/

unsigned WvHash(const UniConfDaemonWatch &watch)
{
    return WvHash(watch.key) ^ unsigned(watch.depth);
}



/***** UniConfDaemonConn *****/

UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, const UniConf &_root) :
    UniClientConn(_s),
    root(_root), watches(113) /*FIXME: embedded tuning parameter */
{
    writecmd(EVENT_HELLO, "{UniConf Server ready}");
}


UniConfDaemonConn::~UniConfDaemonConn()
{
    // clear all watches
    UniConfDaemonWatchTable::Iter i(watches);
    for (i.rewind(); i.next();)
    {
        log(WvLog::Debug5, "Removing a watch for \"%s\" depth %s\n",
            i->key, UniConfDepth::nameof(i->depth));
        root[i->key].del_callback(wvcallback(UniConfCallback, *this,
            UniConfDaemonConn::deltacallback), NULL, i->depth);
    }
}


void UniConfDaemonConn::execute()
{
    UniClientConn::execute();
    for (;;)
    {
        UniClientConn::Command command = readcmd();
        if (command == UniClientConn::NONE)
            break;

        // parse and execute command
        WvString arg1(readarg());
        WvString arg2(readarg());
        switch (command)
        {
            case UniClientConn::INVALID:
                do_malformed();
                break;
            
            case UniClientConn::REQ_NOOP:
                do_noop();
                break;

            case UniClientConn::REQ_GET:
            {
                if (arg1.isnull())
                    do_malformed();
                else
                    do_get(arg1);
                break;
            }
            
            case UniClientConn::REQ_SET:
                if (arg1.isnull() || arg2.isnull())
                    do_malformed();
                else
                    do_set(arg1, arg2);
                break;

            case UniClientConn::REQ_REMOVE:
                if (arg1.isnull())
                    do_malformed();
                else
                    do_remove(arg1);
                break;

            case UniClientConn::REQ_ZAP:
                if (arg1.isnull())
                    do_malformed();
                else
                    do_zap(arg1);
                break;

            case UniClientConn::REQ_SUBTREE:
                if (arg1.isnull())
                    do_malformed();
                else
                    do_subtree(arg1);
                break;

            case UniClientConn::REQ_ADDWATCH:
                if (arg1.isnull() || arg2.isnull())
                    do_malformed();
                else
                {
                    UniConfDepth::Type depth = UniConfDepth::fromname(arg2);
                    if (depth == -1)
                        do_malformed();
                    else
                        do_addwatch(arg1, depth);
                }
                break;

            case UniClientConn::REQ_DELWATCH:
                if (arg1.isnull() || arg2.isnull())
                    do_malformed();
                else
                {
                    UniConfDepth::Type depth = UniConfDepth::fromname(arg2);
                    if (depth == -1)
                        do_malformed();
                    else
                        do_delwatch(arg1, depth);
                }
                break;

            case UniClientConn::REQ_QUIT:
                do_quit();
                break;

            case UniClientConn::REQ_HELP:
                do_help();
                break;

            default:
                do_malformed();
                break;
        }
    }
}


void UniConfDaemonConn::do_malformed()
{
    writefail("malformed request");
}


void UniConfDaemonConn::do_noop()
{
    writeok();
}


void UniConfDaemonConn::do_get(const UniConfKey &key)
{
    WvString value(root[key].get());
    if (value.isnull())
        writefail();
    else
    {
        writevalue(key, value);
        writeok();
    }
}


void UniConfDaemonConn::do_set(const UniConfKey &key, WvStringParm value)
{
    if (root[key].set(value))
        writeok();
    else
        writefail();
}


void UniConfDaemonConn::do_remove(const UniConfKey &key)
{
    if (root[key].remove())
        writeok();
    else
        writefail();
}


void UniConfDaemonConn::do_zap(const UniConfKey &key)
{
    if (root[key].zap())
        writeok();
    else
        writefail();
}


void UniConfDaemonConn::do_subtree(const UniConfKey &key)
{
    UniConf cfg(root[key]);
    if (cfg.exists())
    {
        UniConf::Iter it(cfg);
        for (it.rewind(); it.next(); )
            writevalue(it->fullkey(), it->get());
        writeok();
    }
    else
        writefail();
}


void UniConfDaemonConn::do_addwatch(const UniConfKey &key,
    UniConfDepth::Type depth)
{
    UniConfDaemonWatch *watch = new UniConfDaemonWatch(key, depth);
    if (watches[*watch])
    {
        delete watch;
        writefail("already exists");
    }
    else
    {
        log(WvLog::Debug5, "Adding a watch for \"%s\" depth %s\n",
            key, UniConfDepth::nameof(depth));
        watches.add(watch, true);
        root[key].add_callback(wvcallback(UniConfCallback, *this,
            UniConfDaemonConn::deltacallback), NULL, depth);
        writeok();
    }
}


void UniConfDaemonConn::do_delwatch(const UniConfKey &key,
    UniConfDepth::Type depth)
{
    UniConfDaemonWatch watch(key, depth);
    if (watches[watch])
    {
        log(WvLog::Debug5, "Removing a watch for \"%s\" depth %s\n",
            key, UniConfDepth::nameof(depth));
        watches.remove(& watch);
        root[key].del_callback(wvcallback(UniConfCallback, *this,
            UniConfDaemonConn::deltacallback), NULL, depth);
        writeok();
    }
    else
    {
        writefail("no such watch");
    }
}


void UniConfDaemonConn::do_quit()
{
    writeok();
    close();
}


void UniConfDaemonConn::do_help()
{
    for (int i = 0; i < UniClientConn::NUM_COMMANDS; ++i)
        writetext(UniClientConn::cmdinfos[i].description);
    writeok();
}


void UniConfDaemonConn::deltacallback(const UniConf &key, void *userdata)
{
    writecmd(UniClientConn::EVENT_FORGET, wvtcl_escape(key.fullkey()));
}
