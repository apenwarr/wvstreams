
#ifndef __UNICONFDAEMON_H
#define __UNICONFDAEMON_H

#include "strutils.h"

#include "wvlog.h"
#include "wvunixsocket.h"
#include "wvstreamlist.h"
#include "unievents.h"
#include "uniconfini.h"
#include "uniconf.h"
#include "wvtclstring.h"

class UniConfDaemonConn;
class UniConfDaemon
{
public:
    UniConfDaemon();
    ~UniConfDaemon();
    UniConf *domount(WvString mode, WvString file, WvString mp);
    void alertmodified();
    void run();

    bool want_to_die;
    WvLog log;
    UniConf mainconf;
    UniConfNotifier notifier;
    UniConfEvents events;
    bool keymodified;
protected:
    void connection_callback(WvStream &s, void *userdata);
    void accept_connection(WvStream *s);

    void doget(WvString key, UniConfDaemonConn *s);
    void dosubtree(WvString key, UniConfDaemonConn *s);
    void dorecursivesubtree(WvString key, UniConfDaemonConn *s);
    void doset(WvString key, WvConstStringBuffer &fromline, UniConfDaemonConn *s);
    void dook(const WvString cmd, const WvString key, UniConfDaemonConn *s);
    void registerforchange(WvString key, UniConfDaemonConn *s);
    void keychanged(void *userdata, UniConf &conf);
    void update_callbacks(WvString key, UniConfDaemonConn *s, bool one_shot=false);
    void del_callback(WvString key, UniConfDaemonConn *s);
    void add_callback(WvString key, UniConfDaemonConn *s, bool one_shot);

private:
    void errorcheck(WvStream *s, WvString type);
    
    WvStreamList l;
    static const WvString DEFAULT_CONFIG_FILE;
   
    //Temporary friendship for refactoring of methods from UniConfDaemonConn
    //into UniConfDaemon
    friend class UniConfDaemonConn;
};

#endif // __UNICONFDAEMON_H
