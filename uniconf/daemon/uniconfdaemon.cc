/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "wvtcp.h"
#include <signal.h>

const WvString UniConfDaemon::DEFAULT_CONFIG_FILE = "uniconf.ini";
// Daemon

UniConfDaemon::UniConfDaemon() 
    : log("UniConfDaemon"), 
	notifier(mainconf), events(mainconf, "UniConfDaemon")
{
    want_to_die = false;
    keymodified = false;
}

UniConfDaemon::~UniConfDaemon()
{
    // nothing special
}

// Look after the actual mounting, where mode indicates the type of config file
// we are using, file is the location of the actual file, and mp is the point
// where we want to mount the contents into the config tree.
UniConf *UniConfDaemon::domount(WvString mode, WvString mountfrom, WvString mp)
{
    log(WvLog::Debug2, "Attempting to mount the %s file %s to point:  %s.\n",
            mode, mountfrom, mp);
    UniConf *mounted = &mainconf[mp];
    if (mode == "ini")
    {
/*        mounted->generator = new UniConfIniFile(mounted, mountfrom, false);
        mounted->generator->load();*/
        new UniConfIniFile(mounted, mountfrom, true);
        return mounted;
    }
    else
        return NULL;
}




void UniConfDaemon::errorcheck(WvStream *s, WvString type)
{
    if (!s || !s->isok())
    {
        log(WvLog::Error, "ERROR: Stream type %s could not be created.\n", type);
        log(WvLog::Error, "REASON: %s\n", s->errstr());
        exit(2);
    }
}

// Daemon looks after running
void UniConfDaemon::run()
{
    domount("ini", DEFAULT_CONFIG_FILE, "/");

    // Make sure that everything was cleaned up nicely before.
    system("mkdir -p /tmp/uniconf");
    system("rm -fr /tmp/uniconf/uniconfsocket");
    
    // Now listen on our unix socket.
    WvUnixListener *list = new WvUnixListener(WvUnixAddr("/tmp/uniconf/uniconfsocket"), 0755);
    errorcheck(list, "WvUnixListener");

    l.append(list, true); 

    // Now listen on the correct TCP port
    WvTCPListener *tlist = new WvTCPListener(WvIPPortAddr("0.0.0.0", 4111));
    errorcheck(tlist, "WvTCPListener");

    l.append(tlist, true);

    // Now run the actual daemon.
    log(WvLog::Debug2, "Uniconf Daemon starting.\n");
    while (!want_to_die)
    {
        if (list->select(0))
        {
            log("Incoming Connection on unix domain listener.\n");
	    WvStream *s = list->accept();
	    if (s)
		l.append(new UniConfDaemonConn(s, this), true);
        }
        if (tlist->select(0))
        {
            log("Incoming connection on TCP listener.\n");
	    WvStream *s = tlist->accept();
	    if (s)
		l.append(new UniConfDaemonConn(s, this), true);
        }
        
        if (l.select(5000))
            l.callback();
        if (keymodified)
        {
	    notifier.run();
            keymodified = false;
        }
//        log("There are %s streams in my list.\n", l.count());
    }

    // Make sure all of our listeners are closed
    list->close();
    tlist->close();
    
    // Save any changes
    mainconf.save();
}
