/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session.
 */
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "wvtclstring.h"


/***** UniConfDaemonConn *****/

UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, const UniConf &_root)
    : UniClientConn(_s), root(_root)
{
    addcallback();
    writecmd(EVENT_HELLO, wvtcl_escape("UniConf Server ready."));
}


UniConfDaemonConn::~UniConfDaemonConn()
{
    close();
    delcallback();
}


void UniConfDaemonConn::close()
{
    UniClientConn::close();
}


void UniConfDaemonConn::addcallback()
{
    root.add_callback(this, UniConfCallback(this,
		    &UniConfDaemonConn::deltacallback), true);
}


void UniConfDaemonConn::delcallback()
{
    root.del_callback(this, true);
}


void UniConfDaemonConn::execute()
{
    UniClientConn::execute();
    
    for (;;)
    {
        UniClientConn::Command command = readcmd();
        if (command == UniClientConn::NONE)
            break;

        // parse and execute command
        WvString arg1(readarg());
        WvString arg2(readarg());
        switch (command)
        {
	case UniClientConn::NONE:
	    break;
	    
	case UniClientConn::INVALID:
	    do_malformed();
	    break;
            
	case UniClientConn::REQ_NOOP:
	    do_noop();
	    break;
	    
	case UniClientConn::REQ_GET:
	    if (arg1.isnull())
		do_malformed();
	    else
		do_get(arg1);
	    break;
            
	case UniClientConn::REQ_SET:
	    if (arg1.isnull() || arg2.isnull())
		do_malformed();
	    else
		do_set(arg1, arg2);
	    break;
	    
	case UniClientConn::REQ_REMOVE:
	    if (arg1.isnull())
		do_malformed();
	    else
		do_remove(arg1);
	    break;
	    
	case UniClientConn::REQ_SUBTREE:
	    if (arg1.isnull())
		do_malformed();
	    else
		do_subtree(arg1, arg2.num() == 1);
	    break;
	    
	case UniClientConn::REQ_HASCHILDREN:
	    if (arg1.isnull())
		do_malformed();
	    else
		do_haschildren(arg1);
	    break;
	    
	case UniClientConn::REQ_QUIT:
	    do_quit();
	    break;
	    
	case UniClientConn::REQ_HELP:
	    do_help();
	    break;
	    
	default:
	    do_malformed();
	    break;
        }
    }
}


void UniConfDaemonConn::do_malformed()
{
    writefail("malformed request");
}


void UniConfDaemonConn::do_noop()
{
    writeok();
}


void UniConfDaemonConn::do_reply(WvStringParm reply)
{
    writefail("unexpected reply");
}


void UniConfDaemonConn::do_get(const UniConfKey &key)
{
    WvString value(root[key].getme());
    if (value.isnull())
        writefail();
    else
        writeonevalue(key, value);
}


void UniConfDaemonConn::do_set(const UniConfKey &key, WvStringParm value)
{
    root[key].setme(value);
}


void UniConfDaemonConn::do_remove(const UniConfKey &key)
{
    root[key].remove();
}


void UniConfDaemonConn::do_subtree(const UniConfKey &key, bool recursive)
{
    UniConf cfg(root[key]);
    if (cfg.exists())
    {
	if (recursive)
	{
	    UniConf::RecursiveIter it(cfg);
	    for (it.rewind(); it.next(); )
		writevalue(it->fullkey(cfg), it->getme());
	}
	else
	{
	    UniConf::Iter it(cfg);
	    for (it.rewind(); it.next(); )
		writevalue(it->fullkey(cfg), it->getme());
	}
	writeok();
    }
    else
        writefail();
}


void UniConfDaemonConn::do_haschildren(const UniConfKey &key)
{
    bool haschild = root[key].haschildren();
    WvString msg("%s %s", wvtcl_escape(key), haschild ? "TRUE" : "FALSE");
    writecmd(REPLY_CHILD, msg);
}


void UniConfDaemonConn::do_quit()
{
    writeok();
    close();
}


void UniConfDaemonConn::do_help()
{
    for (int i = 0; i < UniClientConn::NUM_COMMANDS; ++i)
        writetext(UniClientConn::cmdinfos[i].description);
    writeok();
}


void UniConfDaemonConn::deltacallback(const UniConf &cfg, const UniConfKey &key)
{
    // for now, we just send notifications for *any* key that changes.
    // Eventually we probably want to do something about having each
    // connection specify exactly which keys it cares about.
    WvString value(cfg[key].getme());
    WvString msg;

    UniConfKey fullkey(cfg.fullkey(cfg));
    fullkey.append(key);

    if (value.isnull())
        msg = WvString("%s", wvtcl_escape(fullkey));
    else
        msg = WvString("%s %s", wvtcl_escape(fullkey),
                                wvtcl_escape(cfg[key].getme()));

    writecmd(UniClientConn::EVENT_NOTICE, msg);
}
