
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "uniconfiter.h"
#include "wvtclstring.h"


UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source)
    : UniConfConn(_s), 
      log(WvString("UniConf to %s", *_s->src()), WvLog::Debug)
{
    print("HELLO UniConf Server ready\n");
    source = _source;
}


UniConfDaemonConn::~UniConfDaemonConn()
{
    WvStringList::Iter i(keys);
    for (i.rewind(); i.next();)
    {
        WvString key = *i;
        if (!key) key = "/";
        source->del_callback(key, this);
    }
}

void UniConfDaemonConn::execute()
{
    UniConfConn::execute();
}
