
#ifndef __WV_UCONFD_H
#define __WV_UCONFD_H

#include <strutils.h>
#include <signal.h>
#include <netinet/in.h>

//#include "uniconfconn.h"
#include "uniconfini.h"
#include "uniconf.h"
#include "wvdiriter.h"
#include "wvfile.h"
#include "wvtclstring.h"
#include <wvlist.h>
#include <wvstreamlist.h>
#include <wvstreamclone.h>
#include <wvlog.h>
#include <wvunixsocket.h>
#include <wvaddr.h>

DeclareWvDict(notifystruct, WvString, key);

class UniConfDaemon
{
public:
    UniConfDaemon();
    ~UniConfDaemon();
    UniConf *domount(WvString mode, WvString file, WvString mp);
    void run();
    void addstream(WvStream *s);

    bool want_to_die;
    WvLog log;
    UniConf mainconf;
protected:
    void handlerequest(WvStream &s, void *userdata);
private:
    WvStreamList *l;
    static const WvString DEFAULT_CONFIG_FILE;
    // For when we want to link daemons together, we can
    // add a tcp connection to our parent here, and pass it as a parameter
    // to main.
};

#endif
