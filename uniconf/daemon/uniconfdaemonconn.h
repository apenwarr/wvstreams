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

    void appendkey(WvString *key) { keys.append(key, true); }

    WvLog log;
    UniConfDaemon *source;
    WvStringList keys;
};

#endif // __UNICONFDAEMONCONN_H
