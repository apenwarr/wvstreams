#ifndef __UNICONFDAEMONCONN_H
#define __UNICONFDAEMONCONN_H

#include "uniconfconn.h"
#include "wvstringlist.h"
#include "wvlog.h"

class UniConfDaemon;

class UniConfDaemonConn : public UniConfConn 
{
public:
    UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source);
    virtual ~UniConfDaemonConn();
    virtual void execute();
protected:
    void doget(WvString key);
    void dosubtree(WvString key);
    void dorecursivesubtree(WvString key);
    void doset(WvString key, WvConstStringBuffer &fromline);
    void dook(WvString cmd, WvString key) { print("OK %s %s\n", cmd, key); }
    WvLog log;
    UniConfDaemon *source;
    WvStringList keys;
};

#endif // __UNICONFDAEMONCONN_H
