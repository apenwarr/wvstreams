/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "wvtcp.h"
#include "uniconfiter.h"
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
    UniConf *mounted = &mainconf[mp];
    if (mode == "ini")
    {
        log(WvLog::Debug2, "Attempting to mount the %s file %s to point:  %s->\n",
                mode, mountfrom, mp);
        new UniConfIniFile(mounted, mountfrom, true);
        if (!mounted->checkgen())
        {
            log(WvLog::Error, "IniFile generator was not successfully created for %s with file %s->\n",
                    mp, mountfrom);
            mounted->unmount();
        }
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


/*
 * Methods for actual data retrieval/manipulation via connections begin here
 */

WvString UniConfDaemon::create_return_string(WvString key)
{
    WvString result("%s %s", UniConfConn::UNICONF_RETURN, wvtcl_escape(key));
    if (!!mainconf.get(key))
        result.append(" %s", wvtcl_escape(mainconf.get(key)));
    result.append("\n");

    return result;
}


void UniConfDaemon::dook(const WvString cmd, const WvString key, UniConfDaemonConn *s)
{
    if (s->isok())
        s->print("%s %s %s\n", UniConfConn::UNICONF_OK, cmd, key); 
}

void UniConfDaemon::doget(WvString key, UniConfDaemonConn *s)
{
    dook(UniConfConn::UNICONF_GET, key, s);

    if (s->isok())
    {
        s->print(create_return_string(key));

        // Ensure no duplication of events.
        update_callbacks(key, s);
    }
}

void UniConfDaemon::dosubtree(WvString key, UniConfDaemonConn *s)
{
    UniConf *nerf = &mainconf[key];
    WvString send("%s %s ", UniConfConn::UNICONF_SUBTREE_RETURN, wvtcl_escape(key));
    
    dook(UniConfConn::UNICONF_SUBTREE, key, s);
    
    if (nerf)
    {
        UniConf::Iter i(*nerf);
        for (i.rewind(); i.next();)
        {
            send.append("{%s %s} ", wvtcl_escape(i->name),
                    wvtcl_escape(*i));
            
            update_callbacks(key, s);
       }
    }
   
    send.append("\n");
    if (s->isok())
        s->print(send);
}

void UniConfDaemon::dorecursivesubtree(WvString key, UniConfDaemonConn *s)
{
    UniConf *nerf = &mainconf[key];
    WvString send("%s %s ", UniConfConn::UNICONF_SUBTREE_RETURN, wvtcl_escape(key));
    
    dook(UniConfConn::UNICONF_RECURSIVESUBTREE, key, s);
    
    if (nerf)
    {
        UniConf::RecursiveIter i(*nerf);
        for (i.rewind(); i.next();)
        {
            send.append("{%s %s} ", wvtcl_escape(i->full_key(nerf)),
                    wvtcl_escape(*i));
            
            update_callbacks(key, s);
        }
    }
    send.append("\n");
    if (s->isok())
        s->print(send);
}

void UniConfDaemon::doset(WvString key, WvConstStringBuffer &fromline, UniConfDaemonConn *s)
{
    WvString newvalue = wvtcl_getword(fromline);
    mainconf[key] = wvtcl_unescape(newvalue);
    keymodified = true;
    dook(UniConfConn::UNICONF_SET, key, s);
}

/*
 * Methods for actual data retrieval/manipulation via connections end here
 */

/*
 * Callback related methods for UniConfKeys begin here
 */
void UniConfDaemon::keychanged(void *userdata, UniConf &conf)
{
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;
    
    UniConfDaemonConn *s = (UniConfDaemonConn *)userdata;
    WvString keyname(conf.gen_full_key()); 

    if (s->isok())
        s->print(create_return_string(keyname));
}

void UniConfDaemon::update_callbacks(WvString key, UniConfDaemonConn *s, bool one_shot)
{
    del_callback(key, s);
    add_callback(key, s, one_shot);
}

void UniConfDaemon::del_callback(WvString key, UniConfDaemonConn *s)
{
    events.del(wvcallback(UniConfCallback, *this,
                UniConfDaemon::keychanged), s, key);

}

void UniConfDaemon::add_callback(WvString key, UniConfDaemonConn *s, bool one_shot)
{
    events.add(wvcallback(UniConfCallback, *this, 
                UniConfDaemon::keychanged), s, key, one_shot);

    // Let the connection track what keys it knows about.
    s->appendkey(new WvString(key));
}

void UniConfDaemon::registerforchange(WvString key, UniConfDaemonConn *s)
{
    dook(UniConfConn::UNICONF_REGISTER, key, s);
    update_callbacks(key, s);
}

/*
 * Callback related methods for UniConfKeys end here
 */

// Look after all of the handling of incoming connections
void UniConfDaemon::connection_callback(WvStream &stream, void *userdata)
{
    UniConfDaemonConn *s = (UniConfDaemonConn *) &stream;
    WvString line, cmd;
   
    s->fillbuffer();

    while (!(line = s->gettclline()).isnull())
    {
        WvConstStringBuffer fromline(line);

        while (!(cmd = wvtcl_getword(fromline)).isnull())
        {
            // check the command
	    if (cmd == UniConfConn::UNICONF_HELP)
	    {
                if (s->isok())
	    	    s->print("OK I know how to: help, get, subt, quit\n");
		return;	    
	    }
            if (cmd == UniConfConn::UNICONF_QUIT)
            {
                dook(cmd, "<null>", s);
                s->close();
                return;
            }
            WvString key = wvtcl_getword(fromline);
            if (key.isnull())
                break;

            if (cmd == UniConfConn::UNICONF_GET)
            {
                doget(key, s);
            }
            else if (cmd == UniConfConn::UNICONF_SUBTREE)
            {
                dosubtree(key, s);
            }
            else if (cmd == UniConfConn::UNICONF_RECURSIVESUBTREE)
            {
                dorecursivesubtree(key, s);
            }
            else if (cmd == UniConfConn::UNICONF_SET) 
            {
                doset(key, fromline, s);
            }
            else if (cmd == UniConfConn::UNICONF_REGISTER)
            {
                registerforchange(key, s);
            }
        }
    }
}

void UniConfDaemon::accept_connection(WvStream *stream)
{
    if (stream)
    {
        UniConfDaemonConn *s = new UniConfDaemonConn(stream, this);
        s->setcallback(wvcallback(WvStreamCallback, *this, UniConfDaemon::connection_callback), NULL);
        l.append(s, true);
    }
    else
        log(WvLog::Debug2, "Incoming connection was null.\n");
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
            accept_connection(list->accept());
        }
        if (tlist->select(0))
        {
            log("Incoming connection on TCP listener.\n");
            accept_connection(tlist->accept());
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
