
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

    WvString create_return_string(WvString key);
    
    void alertmodified();
    void run();
    void doget(WvString key, UniConfDaemonConn *s);
    void dosubtree(WvString key, UniConfDaemonConn *s);
    void dorecursivesubtree(WvString key, UniConfDaemonConn *s);
    void doset(WvString key, WvConstStringBuffer &fromline, UniConfDaemonConn *s);
    void dook(const WvString cmd, const WvString key, UniConfDaemonConn *s);
    void registerforchange(WvString key, UniConfDaemonConn *s);
    void myvaluechanged(void *userdata, UniConf &conf);
    void me_or_imm_child_changed(void *userdata, UniConf &conf);
    void me_or_any_child_changed(void *userdata, UniConf &conf);

    // The depth parameter is for:  
    // 0 - notify me only if my value has changed
    // 1 - notify me if my value or any of my immediate children have changed
    // 2 - notify me if my value or any of my children have changed
    void update_callbacks(WvString key, UniConfDaemonConn *s, bool one_shot=false, int depth=0);
    void del_callback(WvString key, UniConfDaemonConn *s, int depth=0);
    void add_callback(WvString key, UniConfDaemonConn *s, bool one_shot, int depth);

    bool want_to_die;
    WvLog log;
    UniConf mainconf;
    UniConfNotifier notifier;
    UniConfEvents events;
    bool keymodified;
protected:
    void connection_callback(WvStream &s, void *userdata);
    void accept_connection(WvStream *s);

private:
    void errorcheck(WvStream *s, WvString type);
    
    WvStreamList l;
    WvStringList modifiedkeys;

    static const WvString DEFAULT_CONFIG_FILE;
};

#endif // __UNICONFDAEMON_H
