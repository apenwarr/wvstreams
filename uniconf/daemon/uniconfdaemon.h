
#ifndef __UNICONFDAEMON_H
#define __UNICONFDAEMON_H

#include "strutils.h"
//#include <netinet/in.h>

#include "wvlog.h"
#include "wvunixsocket.h"
#include "wvstreamlist.h"
#include "unievents.h"
#include "uniconfini.h"
#include "uniconf.h"
#include "wvtclstring.h"

class UniConfDaemon
{
public:
    UniConfDaemon();
    ~UniConfDaemon();
    UniConf *domount(WvString mode, WvString file, WvString mp);
    void alertmodified();
    void run();
    void keychanged(void *userdata, UniConf &conf);
    void addstream(WvStream *s);

    bool want_to_die;
    WvLog log;
    UniConf mainconf;
protected:
    void handlerequest(WvStream &s, void *userdata);
private:
    WvStreamList l;
    static const WvString DEFAULT_CONFIG_FILE;
    UniConfNotifier notifier;
    UniConfEvents events;
    bool keymodified;
    // For when we want to link daemons together, we can
    // add a tcp connection to our parent here, and pass it as a parameter
    // to main.
};

#endif // __UNICONFDAEMON_H
