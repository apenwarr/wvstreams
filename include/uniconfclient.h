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
#include "wvlog.h"
#include "wvstream.h"
#include <wvtclstring.h>
#include <wvstreamclone.h>

struct waitingdata
{
    WvString key, value;
    waitingdata(WvString _key, WvString _value) : key(_key), value(_value) {};
};

DeclareWvDict(waitingdata, UniConfString, key);


class UniConfClient : public UniConfGen, WvStreamClone
{
public:
//    WvStream *daemoncon;
    UniConf *top;
    WvLog log;
    waitingdataDict dict;
    
    UniConfClient(UniConf *_top, WvStream *conn);

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
    virtual void update(UniConf *&h);

    virtual void save();
protected:
    void execute();
    void savesubtree(UniConf *tree, UniConfKey key);
};


#endif // __UNICONFCLIENT_H
