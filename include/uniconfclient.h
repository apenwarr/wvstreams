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
#include "uniconfconnfactory.h"
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
    UniConfConnFactory *fctry;
    UniConfConn *conn;
    WvLog log;
    waitingdataDict dict;
    
    UniConfClient(UniConf *_top, UniConfConnFactory *_fctry);
    ~UniConfClient();

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
    virtual void enumerate_subtrees(UniConf *conf);
    virtual void update(UniConf *&h);
    virtual bool deleteable();
    virtual bool isok() { return conn->isok(); }
    virtual void save();
protected:
    void execute();
    void savesubtree(UniConf *tree, UniConfKey key);
    bool waitforsubt;
private:
    unsigned long references;
};
/*
class UniConfViaClient : public UniConf
{
public:
    UniConfViaClient(WvIPPortAddr addr);
    UniConfViaClient(WvIPAddr addr);
    UniConfViaClient(WvUnixAddr addr);
    ~UniConfViaClient();
protected:
private:
};
*/
#endif // __UNICONFCLIENT_H
