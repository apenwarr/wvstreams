
#ifndef __WV_UCONFD_H
#define __WV_UCONFD_H

#include <strutils.h>
#include <signal.h>
#include <netinet/in.h>

#include <unievents.h>
#include <wvstreamlist.h>
#include <wvlog.h>
#include <wvunixsocket.h>
#include <wvaddr.h>
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
    WvStreamList *l;
    static const WvString DEFAULT_CONFIG_FILE;
    UniConfEvents *events;
    bool keymodified;
    // For when we want to link daemons together, we can
    // add a tcp connection to our parent here, and pass it as a parameter
    // to main.
};

#endif
