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
    UniConfClient(UniConf *_top, WvStream *stream, bool automount);
    ~UniConfClient();

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
    virtual void enumerate_subtrees(UniConf *conf);
    virtual void update(UniConf *&h);
    virtual bool deleteable();
    virtual bool isok();
    virtual void save();
protected:
    void execute();
    void savesubtree(UniConf *tree, UniConfKey key);
    bool waitforsubt;
private:
    unsigned long references;
};
#endif // __UNICONFCLIENT_H
