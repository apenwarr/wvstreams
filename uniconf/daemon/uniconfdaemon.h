
#ifndef __UNICONFDAEMON_H
#define __UNICONFDAEMON_H

#include "strutils.h"

#include "wvlogrcv.h"
#include "wvlog.h"
#include "wvstreamlist.h"
#include "uniconf.h"
#include "wvtclstring.h"

class UniConfDaemonConn;
class UniConfDaemon
{
public:
    UniConfDaemon(WvLog::LogLevel level = WvLog::Info);
    ~UniConfDaemon();
    UniConf *domount(const UniConfKey &mountpoint,
        const UniConfLocation &location);

    WvString create_return_string(const UniConfKey &key);
    
    void alertmodified();
    void run();
    void doget(const UniConfKey &key, UniConfDaemonConn *s);
    void dosubtree(const UniConfKey &key, UniConfDaemonConn *s);
    void dorecursivesubtree(const UniConfKey &key,
        UniConfDaemonConn *s);
    void doset(const UniConfKey &key,
        WvConstStringBuffer &fromline, UniConfDaemonConn *s);
    void dook(const WvString cmd, const UniConfKey &key,
        UniConfDaemonConn *s);
    void dofail(const WvString cmd, const UniConfKey &key,
        UniConfDaemonConn *s);
    void registerforchange(const UniConfKey &,
        UniConfDaemonConn *s);
    void deletesubtree(const UniConfKey &, UniConfDaemonConn *s);
    void myvaluechanged(UniConf &conf, void *userdata);
    void me_or_imm_child_changed(UniConf &conf, void *userdata);
    void me_or_any_child_changed(UniConf &conf, void *userdata);

    // The depth parameter is for:  
    // 0 - notify me only if my value has changed
    // 1 - notify me if my value or any of my immediate children have changed
    // 2 - notify me if my value or any of my children have changed
    void update_callbacks(const UniConfKey &key,
        UniConfDaemonConn *s, bool one_shot = false, int depth = 0);
    void del_callback(const UniConfKey &key,
        UniConfDaemonConn *s, int depth = 0);
    void add_callback(const UniConfKey &key,
        UniConfDaemonConn *s, bool one_shot, int depth);

    bool want_to_die;
    WvLog log;
    UniConf mainconf;
    bool keymodified;

protected:
    void connection_callback(WvStream &s, void *userdata);
    void accept_connection(WvStream *s);

private:
    void errorcheck(WvStream *s, WvString type);
    WvLogConsole logcons;
    
    WvStreamList l;
    UniConfKeyList modifiedkeys;

    void dolog(WvLog::LogLevel level,
        WvStringParm func, WvStringParm msg)
    {
        log(level, "UniConfDaemon::%s -> %s.\n",func, msg);
    }

    static const WvString DEFAULT_CONFIG_FILE;
    static const WvString ENTERING, LEAVING;
};

#endif // __UNICONFDAEMON_H
