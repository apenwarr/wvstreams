/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session.
 */
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "wvtclstring.h"
#include "wvstrutils.h"


/***** UniConfDaemonConn *****/

UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, const UniConf &_root)
    : UniClientConn(_s), root(_root)
{
    uses_continue_select = true;
    addcallback();
    writecmd(EVENT_HELLO,
	     spacecat(wvtcl_escape("UniConf Server ready."),
		      wvtcl_escape(UNICONF_PROTOCOL_VERSION)));
}


UniConfDaemonConn::~UniConfDaemonConn()
{
    close();
    terminate_continue_select();
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

    WvString command_string;
    UniClientConn::Command command = readcmd(command_string);
    if (command != UniClientConn::NONE)
    {
        // parse and execute command
        WvString arg1(readarg());
        WvString arg2(readarg());
        switch (command)
        {
	case UniClientConn::NONE:
	    break;
	    
	case UniClientConn::INVALID:
	    do_invalid(command_string);
	    break;
            
	case UniClientConn::REQ_NOOP:
	    do_noop();
	    break;
	    
	case UniClientConn::REQ_GET:
	    if (arg1.isnull())
		do_malformed(command);
	    else
		do_get(arg1);
	    break;
            
	case UniClientConn::REQ_SET:
	    if (arg1.isnull() || arg2.isnull())
		do_malformed(command);
	    else
		do_set(arg1, arg2);
	    break;
	    
	case UniClientConn::REQ_REMOVE:
	    if (arg1.isnull())
		do_malformed(command);
	    else
		do_remove(arg1);
	    break;
	    
	case UniClientConn::REQ_SUBTREE:
	    if (arg1.isnull())
		do_malformed(command);
	    else
		do_subtree(arg1, arg2.num() == 1);
	    break;
	    
	case UniClientConn::REQ_HASCHILDREN:
	    if (arg1.isnull())
		do_malformed(command);
	    else
		do_haschildren(arg1);
	    break;
	    
	case UniClientConn::REQ_COMMIT:
            do_commit();
	    break;
	    
	case UniClientConn::REQ_REFRESH:
            do_refresh();
	    break;
	    
	case UniClientConn::REQ_QUIT:
	    do_quit();
	    break;
	    
	case UniClientConn::REQ_HELP:
	    do_help();
	    break;
	    
	default:
	    do_invalid(command_string);
	    break;
        }
    }
}


void UniConfDaemonConn::do_invalid(WvStringParm c)
{
    writefail(WvString("unknown command: %s", c));
}


void UniConfDaemonConn::do_malformed(UniClientConn::Command c)
{
    writefail(WvString("malformed request: %s",
		       UniClientConn::cmdinfos[c].name));
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
    static int niceness = 0;
    
    UniConf cfg(root[key]);
    if (cfg.exists())
    {
	if (recursive)
	{
	    UniConf::RecursiveIter it(cfg);
	    for (it.rewind(); it.next(); )
	    {
		writevalue(it->fullkey(cfg), it._value());
		
		// the output might be totally gigantic.  Don't hog the
		// entire daemon while fulfilling it; give up our timeslice
		// after each entry.
		if (!isok()) break;
		if (++niceness > 100)
		{
		    niceness = 0;
		    continue_select(0);
		}
	    }
	}
	else
	{
	    UniConf::Iter it(cfg);
	    for (it.rewind(); it.next(); )
	    {
		writevalue(it->fullkey(cfg), it._value());
		
		// the output might be totally gigantic.  Don't hog the
		// entire daemon while fulfilling it; give up our timeslice
		// after each entry.
		if (!isok()) break;
		continue_select(0);
	    }
	}
	writeok();
    }
    else
        writefail();
}


void UniConfDaemonConn::do_haschildren(const UniConfKey &key)
{
    bool haschild = root[key].haschildren();
    writecmd(REPLY_CHILD,
	     spacecat(wvtcl_escape(key), haschild ? "TRUE" : "FALSE"));
}


void UniConfDaemonConn::do_commit()
{
    root.commit();
    writeok();
}


void UniConfDaemonConn::do_refresh()
{
    if (root.refresh())
        writeok();
    else
        writefail();
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
        msg = wvtcl_escape(fullkey);
    else
        msg = spacecat(wvtcl_escape(fullkey),
		       wvtcl_escape(cfg[key].getme()));

    writecmd(UniClientConn::EVENT_NOTICE, msg);
}
