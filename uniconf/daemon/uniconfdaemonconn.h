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
    void doget(WvString key, WvStream *s);
    void dosubtree(WvString key, WvStream *s);
    void dorecursivesubtree(WvString key, WvStream *s);
    void doset(WvString key, WvConstStringBuffer &fromline, WvStream *s);
    void registerforchange(WvString key);
    void dook(const WvString cmd, const WvString key, WvStream *s);
    void add_callback(WvString key, bool one_shot, WvStream *s);
    void del_callback(WvString key, WvStream *s);
    void update_callbacks(WvString key, WvStream *s, bool one_shot=false);
    void keychanged(void *userdata, UniConf &conf);

    WvLog log;
    UniConfDaemon *source;
    WvStringList keys;
};

#endif // __UNICONFDAEMONCONN_H
