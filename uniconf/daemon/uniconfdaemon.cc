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
UniConf *UniConfDaemon::domount(WvString mode, WvString file, WvString mp)
{
    log(WvLog::Debug2, "Attempting to mount the %s file %s to point:  %s.\n",
            mode, file, mp);
    UniConf *mounted = &mainconf[mp];
    if (mode == "ini")
    {
        mounted->generator = new UniConfIniFile(mounted, file);
        mounted->generator->load();
        return mounted;
    }
    else
        return NULL;
}

void UniConfDaemon::keychanged(void *userdata, UniConf &conf)
{
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;
    
    WvStream *s = (WvStream *)userdata;
    // Ok, now compile the complete keyname.
    WvString keyname("/%s", conf.name);
    UniConf *f = conf.parent;
    while (f)
    {
        if (f->name.len() > 0)
            keyname = WvString("/%s%s", f->name, keyname);
        f = f->parent;
    }
    // now notify.
    log(WvLog::Debug2, "Got a callback for key %s.\n", keyname);
    if (s->isok())
        s->write(WvString("FGET %s\n", keyname));
//    events.del(wvcallback(UniConfCallback, *this, UniConfDaemon::keychanged), s, keyname);

}

// Daemon looks after running
void UniConfDaemon::run()
{
//    WvBuffer *newbuf = new WvBuffer;
    // Mount our initial config file.
    // FIXME:  When we decide to mount configurations from various locations,
    // the domount command can be used, combined with data read in from the
    // configuration file which tells us where to find information.
    domount("ini", DEFAULT_CONFIG_FILE, "/");

    // Make sure that everything was cleaned up nicely before.
    system("mkdir -p /tmp/uniconf");
    system("rm -fr /tmp/uniconf/uniconfsocket");
    
    // Now listen on our unix socket.
    WvUnixListener *list = new WvUnixListener(WvUnixAddr("/tmp/uniconf/uniconfsocket"), 0755);
    if (!list->isok())
    {
        log(WvLog::Error, "ERROR:  WvUnixListener could not be created.\n");
        log(WvLog::Error, "Error Reason:  %s\n", list->errstr());
        exit(2);
    }
    l.append(list, true); 

    // Now listen on the correct TCP port
    WvTCPListener *tlist = new WvTCPListener(WvIPPortAddr("0.0.0.0", 4111));
    if (!tlist->isok())
    {
        log(WvLog::Error,"ERROR:  WvTCPListener could not be created.\n");
        log(WvLog::Error,"Error Reason:  %s\n", tlist->errstr());
        exit(2);
    }
    l.append(tlist, true);

    // Now run the actual daemon.
    log(WvLog::Debug2, "Uniconf Daemon starting.\n");
    while (!want_to_die)
    {
        if (list->select(0, true, false))
            l.append(new UniConfDaemonConn(list->accept(), this), true);
        if (tlist->select(0, true, false))
            l.append(new UniConfDaemonConn(tlist->accept(), this), true);
        
        if (l.select(5000, true, false))
            l.callback();
        if (keymodified)
	    notifier.run();
    }

    // Make sure all of our listeners are closed
    list->close();
    tlist->close();
    // Save any changes
    mainconf.save();
}
