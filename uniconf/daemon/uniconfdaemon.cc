/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */
#include "uniconfdaemon.h"
#include "uniconfdaemonconn.h"
#include "uniconfiter.h"
#include <signal.h>

const WvString UniConfDaemon::DEFAULT_CONFIG_FILE = "uniconf.ini";
const WvString UniConfDaemon::ENTERING = "Entering";
const WvString UniConfDaemon::LEAVING = "Leaving";
// Daemon

UniConfDaemon::UniConfDaemon(WvLog::LogLevel level) :
    log("UniConfDaemon"),
    logcons(1, level)
{
    want_to_die = false;
    keymodified = false;
    mainconf.attach(& l);
}


UniConfDaemon::~UniConfDaemon()
{
    mainconf.detach(& l);
}


UniConf *UniConfDaemon::domount(const UniConfKey &mountpoint,
    const UniConfLocation &location)
{
    dolog(WvLog::Debug1, "domount", ENTERING);
    UniConf *mounted = & mainconf[mountpoint];
    dolog(WvLog::Debug3, "domount", WvString("Mounting "
        "\"%s\" from \"%s\"\n", mountpoint, location));
    if (mounted->mount(location))
        return mounted;

    dolog(WvLog::Error, "domount", WvString("Could not mount "
        "\"%s\" from \"%s\"\n", mountpoint, location));
    return NULL;
}

void UniConfDaemon::errorcheck(WvStream *s, WvString type)
{
    if (!s || !s->isok())
    {
        dolog(WvLog::Error, "errorcheck", WvString("ERROR: Stream type %s could not be created.\n", type));
        dolog(WvLog::Error, "errorcheck", WvString("REASON: %s\n", s->errstr()));
        exit(2);
    }
}


/*
 * Methods for actual data retrieval/manipulation via connections begin here
 */

WvString UniConfDaemon::create_return_string(const UniConfKey &key)
{
    dolog(WvLog::Debug1, "create_return_string", ENTERING);
    dolog(WvLog::Debug3, "create_return_string", WvString("Creating return string for key %s.\n", key));

    WvString result("%s %s", UniConfConn::UNICONF_RETURN,
        wvtcl_escape(key));
    WvString value = mainconf.get(key);
    if (! value.isnull())
        result.append(" %s", wvtcl_escape(value));
    result.append("\n");

    dolog(WvLog::Debug1, "create_return_string", LEAVING);
    return result;
}


void UniConfDaemon::dook(const WvString cmd,
    const UniConfKey &key, UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "dook", ENTERING);
    dolog(WvLog::Debug2, "dook", WvString("Connection:  %s", *s->src()));
    if (s->isok())
    {
        WvString okmsg("%s %s %s\n", UniConfConn::UNICONF_OK, cmd, key);
        dolog(WvLog::Debug3, "dook", WvString("MSG TO %s:  %s\n", *s->src(),okmsg));
        s->print(okmsg);
    }
    dolog(WvLog::Debug1, "dook", LEAVING);
}


void UniConfDaemon::dofail(const WvString cmd,
    const UniConfKey &key, UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "dofail", ENTERING);
    dolog(WvLog::Debug2, "dofail", WvString("Connection:  %s", *s->src()));
    if (s->isok())
    {
        WvString okmsg("%s %s %s\n", UniConfConn::UNICONF_FAIL, cmd, key);
        dolog(WvLog::Debug3, "dofail", WvString("MSG TO %s:  %s\n", *s->src(),okmsg));
        s->print(okmsg);
    }
    dolog(WvLog::Debug1, "dofail", LEAVING);
}


void UniConfDaemon::doget(const UniConfKey &key, UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "doget", ENTERING);
    dolog(WvLog::Debug2, "doget", WvString("Connection:  %s", *s->src()));
    dook(UniConfConn::UNICONF_GET, key, s);

    if (s->isok())
    {
        dolog(WvLog::Debug3, "doget", WvString("Sending:%s TO:%s.", create_return_string(key), *s->src()));
        s->print(create_return_string(key));

        // Ensure no duplication of events.
        update_callbacks(key, s);
    }
    dolog(WvLog::Debug1, "doget", LEAVING);
}


void UniConfDaemon::dosubtree(const UniConfKey &key,
    UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "dosubtree", ENTERING);
    dolog(WvLog::Debug2, "dosubtree",
        WvString("Connection:  %s", *s->src()));
    UniConf *nerf = mainconf.find(key);
    WvString send("%s %s ", UniConfConn::UNICONF_SUBTREE_RETURN,
        wvtcl_escape(key));
    update_callbacks(key, s, false, 1);
    
    dook(UniConfConn::UNICONF_SUBTREE, key, s);
    
    if (nerf)
    {
        UniConf::Iter it(*nerf);
        for (it.rewind(); it.next();)
        {
            if (! it->value().isnull())
            {
                dolog(WvLog::Debug3, "dosubtree",
                    WvString("Sending Key:%s.With val:%s.\n",
                    it->fullkey(), it->value()));
                send.append("{%s %s} ",
                    wvtcl_escape(it->fullkey()),
                    wvtcl_escape(it->value()));
            }
        }
    }
    dolog(WvLog::Debug3, "dosubtree", WvString("Sending:%s TO:%s.", send,  *s->src()));
   
    send.append("\n");
    if (s->isok())
    {
        s->print(send);
    }
    dolog(WvLog::Debug1, "dosubtree", LEAVING);
}


void UniConfDaemon::dorecursivesubtree(const UniConfKey &key,
    UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "dorecursivesubtree", ENTERING);
    dolog(WvLog::Debug2, "dorecursivesubtree",
        WvString("Connection:  %s", *s->src()));
    UniConf *nerf = mainconf.find(key);
    WvString send("%s %s ", UniConfConn::UNICONF_SUBTREE_RETURN,
        wvtcl_escape(key));
    
    dook(UniConfConn::UNICONF_RECURSIVESUBTREE, key, s);
    update_callbacks(key, s, false, 2);
    
    if (nerf)
    {
        UniConf::RecursiveIter it(*nerf);
        for (it.rewind(); it.next();)
        {
            if (! it->value().isnull())
            {
                dolog(WvLog::Debug3, "dorecursivesubtree",
                    WvString("Sending Key:%s.With val:%s.\n",
                    it->key(), it->value()));
                send.append("{%s %s} ",
                    wvtcl_escape(it->fullkey(nerf)),
                    wvtcl_escape(it->value()));
            }
        }
    }
    send.append("\n");
    if (s->isok())
    {
        dolog(WvLog::Debug3, "dorecursivesubtree", WvString("Sending:%s TO:%s",send, *s->src()));
        s->print(send);
    }
    dolog(WvLog::Debug1, "dorecursivesubtree", LEAVING);
}


void UniConfDaemon::doset(const UniConfKey &key,
    WvConstStringBuffer &fromline, UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "doset", ENTERING);
    dolog(WvLog::Debug2, "doset", WvString("Connection:  %s", *s->src()));
   
    WvString newvalue = wvtcl_getword(fromline);
    bool success = mainconf.set(key, wvtcl_unescape(newvalue));
    
    if (success)
    {
        dolog(WvLog::Debug3, "doset",
            WvString("New value for %s %s", key, mainconf.get(key)));
        keymodified = true;
        modifiedkeys.append(new UniConfKey(key), true);
        dook(UniConfConn::UNICONF_SET, key, s);
    }
    else
    {
        dolog(WvLog::Debug3, "doset",
            WvString("Could not set value for %s", key));
        dofail(UniConfConn::UNICONF_SET, key, s);
    }
    dolog(WvLog::Debug1, "doset", LEAVING);
}

/*
 * Methods for actual data retrieval/manipulation via connections end here
 */

/*
 * Callback related methods for UniConfKeys begin here
 */
void UniConfDaemon::myvaluechanged(UniConf &conf, void *userdata)
{
    dolog(WvLog::Debug1, "myvaluechanged", ENTERING);
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;
    
    UniConfDaemonConn *s = (UniConfDaemonConn *)userdata;
    dolog(WvLog::Debug2, "myvaluechanged",
        WvString("Connection:  %s", *s->src()));
    UniConfKey keyname(conf.fullkey()); 

    // FIXME: sending spurious notifications
    if (s->isok())
    {
        dolog(WvLog::Debug3, "myvaluechanged",
            WvString("SENDING: %s.  TO:%s",
            create_return_string(keyname), *s->src()));
        s->print(create_return_string(keyname));
    }
    dolog(WvLog::Debug1, "myvaluechanged", LEAVING);
}


void UniConfDaemon::me_or_imm_child_changed(UniConf &conf,
    void *userdata)
{
    dolog(WvLog::Debug1, "me_or_imm_child_changed", ENTERING);
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;

    UniConfDaemonConn *s = (UniConfDaemonConn *)userdata;
    UniConfKey key = conf.fullkey();

    // FIXME: sending spurious notifications
    WvString response(create_return_string(key.printable()));

    UniConfKeyList::Iter i(modifiedkeys);

    for (i.rewind(); i.next();)
    {
        if (key == i()) // This case has already been handled
            continue;
        
        UniConfKey modkey(i());
        
        bool match = true;
       
        // only have to do the loop if we have a key that's not root. 
        if (! key.isempty())
        {
            UniConfKey::Iter i1(key);
            UniConfKey::Iter i2(modkey);
            for (i1.rewind(), i2.rewind(); i1.next() && i2.next() && match; )
            {
                match = (i1() == i2());
            }
            // Ok, we should have ONE link, then a NULL for this key to matter
            if (! i2.next() || i2.next())
                match = false;
        }
        else // Make sure we're an IMMEDIATE subchild of /
        {
            match = modkey.numsegments() == 1;
        }

        if (match)
        {
            UniConf *subtree = mainconf.find(i());
            // FIXME: sending spurious notifications
            if (subtree)
                response.append(create_return_string(i()));
        }   
    }
    if (s->isok())
    {
        dolog(WvLog::Debug3, "me_or_imm_child_changed",
            WvString("SENDING:%s TO:%s.",response,*s->src()));
        s->print(response);
    }
    dolog(WvLog::Debug1, "me_or_imm_child_changed", LEAVING);
}


void UniConfDaemon::me_or_any_child_changed(UniConf &conf,
    void *userdata)
{
    dolog(WvLog::Debug1, "me_or_any_child_changed", ENTERING);
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;

    UniConfDaemonConn *s = (UniConfDaemonConn *)userdata;
    UniConfKey key = conf.fullkey();

    // FIXME: sending spurious notifications
    WvString response(create_return_string(key.printable()));

    UniConfKeyList::Iter i(modifiedkeys);

    for (i.rewind(); i.next();)
    {
        if (key == i())
            continue;

        UniConfKey modkey(i());
        
        bool match = true;
       
        // only have to do the loop if we have a key that's not root. 
        if (! key.isempty())
        {
            UniConfKey::Iter i1(key);
            UniConfKey::Iter i2(modkey);
            for (i1.rewind(), i2.rewind(); i1.next() && i2.next() && match; )
            {
                match = (i1() == i2());
            }
        }
    
        if (match)
        {
            UniConf *subtree = mainconf.find(i());
            // FIXME: sending spurious notifications
            if (subtree)
                response.append(create_return_string(i()));
        }
    }

    if (s->isok())
    {
        dolog(WvLog::Debug3, "me_or_any_child_changed", WvString("SENDING:%s TO:%s.",response,*s->src()));
        s->print(response);
    }
    dolog(WvLog::Debug1, "me_or_any_child_changed", LEAVING);
}


void UniConfDaemon::update_callbacks(const UniConfKey &key,
    UniConfDaemonConn *s, bool one_shot, int depth)
{
    dolog(WvLog::Debug1, "update_callbacks",ENTERING);
    del_callback(key, s, depth);
    add_callback(key, s, one_shot, depth);
    dolog(WvLog::Debug1, "update_callbacks",LEAVING); 
}


void UniConfDaemon::del_callback(const UniConfKey &key,
    UniConfDaemonConn *s, int depth)
{
    dolog(WvLog::Debug1, "del_callback", ENTERING);
    switch (depth)
    {
        case 0:
            mainconf.delwatch(key, UniConf::ZERO,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::myvaluechanged), s);
            break;
        case 1:
            mainconf.delwatch(key, UniConf::ONE,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::me_or_imm_child_changed), s);
            break;
        case 2:
            mainconf.delwatch(key, UniConf::INFINITE,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::me_or_any_child_changed), s);
            break;
        default:
            dolog(WvLog::Warning, "del_callback", "Attempting to delete call back with unsupported depth");
    }
    dolog(WvLog::Debug1, "del_callback", LEAVING);
}


void UniConfDaemon::add_callback(const UniConfKey &key,
    UniConfDaemonConn *s, bool one_shot, int depth)
{
    dolog(WvLog::Debug1, "add_callback",ENTERING);
    // FIXME: we don't handle oneshot notifications correctly
    switch (depth)
    {
        case 0:
            mainconf.addwatch(key, UniConf::ZERO,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::myvaluechanged), s);
            break;
        case 1:
            mainconf.addwatch(key, UniConf::ONE,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::me_or_imm_child_changed), s);
            break;
        case 2:
            mainconf.addwatch(key, UniConf::INFINITE,
                wvcallback(UniConfCallback, *this,
                UniConfDaemon::me_or_any_child_changed), s);
            break;
        default:
            dolog(WvLog::Warning, "add_callback", "Attempting to add call back with unsupported depth");
    }
    // Let the connection track what keys it knows about.
    s->appendkey(new WvString(key));
    dolog(WvLog::Debug1, "add_callback", LEAVING);
}


void UniConfDaemon::registerforchange(const UniConfKey &key,
    UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "registerforchange", ENTERING);
    dook(UniConfConn::UNICONF_REGISTER, key, s);
    update_callbacks(key, s);
    dolog(WvLog::Debug1, "registerforchange", LEAVING);
}


void UniConfDaemon::deletesubtree(const UniConfKey &key,
    UniConfDaemonConn *s)
{
    dolog(WvLog::Debug1, "deletesubtree", ENTERING);
    dook(UniConfConn::UNICONF_DEL, key, s);
    dolog(WvLog::Debug2, "deletesubtree", WvString("Executing delete command from %s.\n", *s->src()));
    dolog(WvLog::Debug3, "deletesubtree", WvString("Deleting key:  %s.\n", key));
    mainconf.remove(key);
    dolog(WvLog::Debug1, "deletesubtree", LEAVING);
}

/*
 * Callback related methods for UniConfKeys end here
 */

// Look after all of the handling of incoming connections
void UniConfDaemon::connection_callback(WvStream &stream,
    void *userdata)
{
    dolog(WvLog::Debug1, "connection_callback", ENTERING);

    UniConfDaemonConn *s = (UniConfDaemonConn *) &stream;
    
    dolog(WvLog::Debug2, "connection_callback", WvString("Callback for: %s", *s->src()));
    
    WvString line, cmd, known_cmds("help, get, subt, rsub, quit");
   
    s->fillbuffer();

    while (!(line = s->gettclline()).isnull())
    {
        WvConstStringBuffer fromline(line);

        while (!(cmd = wvtcl_getword(fromline)).isnull())
        {
            // check the command
            WvString logstring("RECEIVED:  FROM:%s.  CMD:%s", *s->src(), cmd);
	    if (cmd == UniConfConn::UNICONF_HELP)
	    {
                dolog(WvLog::Debug3, "connection_callback", logstring);
                if (s->isok())
	    	    s->print("OK I know how to: %s\n", known_cmds);
		return;	    
	    }
            if (cmd == UniConfConn::UNICONF_QUIT)
            {
                dolog(WvLog::Debug3, "connection_callback", logstring);
                dook(cmd, "<null>", s);
                s->close();
                return;
            }
            WvString key = wvtcl_getword(fromline);

            if (key.isnull())
            {
                dolog(WvLog::Warning, "connection_callback", WvString("NO KEY SENT FROM %s.\n", *s->src()));
                if (s->isok())
                    s->print("%s %s {NO KEY}.\n", UniConfConn::UNICONF_FAIL, cmd);
                break;
            }
            
            logstring.append(".  KEY:%s", key);

            dolog(WvLog::Debug3, "connection_callback", logstring);
            
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
            else if (cmd == UniConfConn::UNICONF_DEL)
            {
                deletesubtree(key, s);
            }
            else
            {
                dolog(WvLog::Warning, "connection_callback", WvString("Received Unknown Command:  %s", cmd));
                if (s->isok())
                    s->print("FAIL %s {NO KNOWN COMMAND}.  I know:  %s.\n",cmd, known_cmds);
            }
        }
    }
    dolog(WvLog::Debug1, "connection_callback", LEAVING);
}


void UniConfDaemon::accept_connection(WvStream *stream)
{
    WvStringParm myname("accept_connection");
    dolog(WvLog::Debug1, myname, ENTERING);
    if (stream)
    {
        UniConfDaemonConn *s = new UniConfDaemonConn(stream, this);
        dolog(WvLog::Debug2, myname, WvString("Received connection from: %s", *s->src()) );
        s->setcallback(wvcallback(WvStreamCallback, *this, UniConfDaemon::connection_callback), NULL);
        l.append(s, true);
    }
    else
        dolog(WvLog::Warning, myname, "Incoming connection was null");

    dolog(WvLog::Debug1, myname, LEAVING);
}


// Daemon looks after running
void UniConfDaemon::run()
{
    WvStringParm myname("run");

    dolog(WvLog::Debug1, myname, ENTERING);
    domount("/", WvString("ini://%s", DEFAULT_CONFIG_FILE));

    // Make sure that everything was cleaned up nicely before.
    dolog(WvLog::Debug3,myname,"Housecleaning");
    system("mkdir -p /tmp/uniconf");
    // FIXME: THIS IS NOT VERY SAFE!
    system("rm -f /tmp/uniconf/uniconfsocket");
    
    // Now listen on our unix socket.
    WvUnixListener *list = new WvUnixListener(WvUnixAddr("/tmp/uniconf/uniconfsocket"), 0755);
    errorcheck(list, "WvUnixListener");

    l.append(list, true); 

    // Now listen on the correct TCP port
    WvTCPListener *tlist = new WvTCPListener(WvIPPortAddr("0.0.0.0",
        DEFAULT_UNICONF_DAEMON_TCP_PORT));
    errorcheck(tlist, "WvTCPListener");

    l.append(tlist, true);

    // Now run the actual daemon.
    dolog(WvLog::Info, "run", "Uniconf Daemon starting.\n");
    while (!want_to_die)
    {
        if (list->select(0))
        {
            dolog(WvLog::Debug2, myname, "Incoming Connection on unix domain listener.");
            accept_connection(list->accept());
        }
        if (tlist->select(0))
        {
            dolog(WvLog::Debug2, myname, "Incoming connection on TCP listener.");
            accept_connection(tlist->accept());
        }
        
        if (l.select(5000))
            l.callback();
        if (keymodified)
        {
            // FIXME: not sure what to do here now that we're
            //        using the new watch mechanism
            keymodified = false;
            modifiedkeys.zap();
        }
//        log("There are %s streams in my list.\n", l.count());
    }

    // Make sure all of our listeners are closed
    list->close();
    tlist->close();
    
    // Save any changes
    dolog(WvLog::Info, myname, "Saving changes");
    mainconf.commit();
    dolog(WvLog::Debug1, myname, LEAVING);
}
