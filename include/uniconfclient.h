/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 */
#ifndef __UNICONFCLIENT_H
#define __UNICONFCLIENT_H

#include "uniconf.h"
#include "uniconfiter.h"
#include "uniconfconn.h"
#include "wvlog.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvtclstring.h"

struct waitingdata
{
    WvString key, value;
    waitingdata(WvString _key, WvString _value) : key(_key), value(_value) {};
};

DeclareWvDict(waitingdata, UniConfString, key);


class UniConfClient : public UniConfGen
{
public:
    UniConf *top;
    UniConfConn *conn;
    WvLog log;
    waitingdataDict dict;
    
    // pass false to automount if you don't want to automatically set _top's generator to this.
    UniConfClient(UniConf *_top, WvStream *stream, WvStreamList *l = NULL);
    UniConfClient(UniConf *_top, WvStream *stream, bool automount = false);
    ~UniConfClient();

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
    virtual void enumerate_subtrees(UniConf *conf, bool recursive);
    virtual void update(UniConf *&h);
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
#endif // __UNICONFCLIENT_H
