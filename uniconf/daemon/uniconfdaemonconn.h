#ifndef __UNICONFDAEMONCONN_H
#define __UNICONFDAEMONCONN_H

#include "uniconf.h"
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
    void registerforchange(WvString key);
    void dook(const WvString cmd, const WvString key);
    void add_callback(WvString key, bool one_shot);
    void del_callback(WvString key);
    void update_callbacks(WvString key, bool one_shot=false);
    void keychanged(void *userdata, UniConf &conf);

    WvLog log;
    UniConfDaemon *source;
    WvStringList keys;
};

#endif // __UNICONFDAEMONCONN_H
