#ifndef __UNICONFDAEMONCONN_H
#define __UNICONFDAEMONCONN_H

#include "uniconfconn.h"
#include "wvstringlist.h"
#include "unievents.h"

class UniConfDaemon;

class UniConfDaemonConn : public UniConfConn 
{
public:
    UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source);
    virtual ~UniConfDaemonConn();
    virtual void execute();
private:
    UniConfDaemon *source;
    WvStringList keys;
};

#endif // __UNICONFDAEMONCONN_H
