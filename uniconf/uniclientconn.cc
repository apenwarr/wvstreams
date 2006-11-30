/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a connection between the UniConf client and daemon.
 */
#include "uniclientconn.h"
#include "wvaddr.h"
#include "wvtclstring.h"
#include "strutils.h"

/***** UniClientConn *****/

/* This table is _very_ important!!!
 *
 * With UniConf, we promise to never remove or modify the behaviour of
 * any of the commands listed here.  If you want to modify anything,
 * you'd better just add a new command instead.  We keep track of the
 * version of the UniConf protocol by the number of commands supported
 * by the server.
 *
 * @see UniClientConn::Commands
 */
const UniClientConn::CommandInfo UniClientConn::cmdinfos[
    UniClientConn::NUM_COMMANDS] = {
    // requests
    { "noop", "noop: verify that the connection is active" },
    { "get", "get <key>: get the value of a key" },
    { "set", "set <key> <value>: sets the value of a key" },
    { "setv", "setv <key> <value> ...: set multiple key-value pairs" },
    { "del", "del <key>: deletes the key" },
    { "subt", "subt <key> <recurse?>: enumerates the children of a key" },
    { "hchild", "hchild <key>: returns whether a key has children" },
    { "commit", "commit: commits changes to disk" },
    { "refresh", "refresh: refresh contents from disk" },
    { "quit", "quit: kills the session nicely" },
    { "help", "help: returns this help text" },
    { "restrict", "restrict <key>: make all commands and notifications relative to a key" },

    // command completion replies
    { "OK", "OK <payload>: reply on command success" },
    { "FAIL", "FAIL <payload>: reply on command failure" },
    { "CHILD", "CHILD <key> TRUE / FALSE: key has children or not" },
    { "ONEVAL", "ONEVAL <key> <value>: reply to a get" },

    // partial replies
    { "VAL", "VAL <key> <value>: intermediate reply value of a key" },
    { "TEXT", "TEXT <text>: intermediate reply of a text message" },

    // events
    { "HELLO", "HELLO <version> <message>: sent by server on connection" },
    { "NOTICE", "NOTICE <key> <oldval> <newval>: forget key and its children" },
};


UniClientConn::UniClientConn(IWvStream *_s, WvStringParm dst) :
    WvStreamClone(_s),
    log(WvString("UniConf to %s", dst.isnull() && _s->src() ? *_s->src() : WvString(dst)),
    WvLog::Debug5), closed(false), version(-1), payloadbuf("")
{
    log("Opened\n");
}


UniClientConn::~UniClientConn()
{
    close();
}


void UniClientConn::close()
{
    if (!closed)
    {
        closed = true;
        WvStreamClone::close();
        log("Closed\n");
    }
}


WvString UniClientConn::readmsg()
{
    WvString word;
    while ((word = wvtcl_getword(msgbuf,
				 WVTCL_NASTY_NEWLINES,
				 false)).isnull())
    {
	// use lots of readahead to prevent unnecessary runs through select()
	// during heavy data transfers.
        char *line = getline(0, '\n', 20480);
        if (line)
        {
            msgbuf.putstr(line);
            msgbuf.put('\n');
        }
        else
        {
            if (!WvStreamClone::isok())
            {
                // possibly left some incomplete command behind
                msgbuf.zap();
            }
            return WvString::null;
        }
    }
    if (!!word && 0)
	log("Read: %s\n", word);
    return word;
}


void UniClientConn::writemsg(WvStringParm msg)
{
    write(msg);
    write("\n");
    // log("Wrote: %s\n", msg);
}


UniClientConn::Command UniClientConn::readcmd()
{
    WvString buf;
    return readcmd(buf);
}

UniClientConn::Command UniClientConn::readcmd(WvString &command)
{
    WvString msg(readmsg());
    if (msg.isnull())
	return NONE;

    // extract command, leaving the remainder in payloadbuf
    payloadbuf.reset(msg);
    command = readarg();

    if (command.isnull())
	return NONE;

    for (int i = 0; i < NUM_COMMANDS; ++i)
	if (strcasecmp(cmdinfos[i].name, command.cstr()) == 0)
	    return Command(i);
    return INVALID;
}


WvString UniClientConn::readarg()
{
    return wvtcl_getword(payloadbuf);
}


void UniClientConn::writecmd(UniClientConn::Command cmd, WvStringParm msg)
{
    if (msg)
        write(WvString("%s %s\n", cmdinfos[cmd].name, msg));
    else
        write(WvString("%s\n", cmdinfos[cmd].name));
}


void UniClientConn::writeok(WvStringParm payload)
{
    writecmd(REPLY_OK, payload);
}


void UniClientConn::writefail(WvStringParm payload)
{
    writecmd(REPLY_FAIL, payload);
}


void UniClientConn::writevalue(const UniConfKey &key, WvStringParm value)
{
    if (value == WvString::null)
        writecmd(PART_VALUE, wvtcl_escape(key));
    else
        writecmd(PART_VALUE, spacecat(wvtcl_escape(key), wvtcl_escape(value)));
}


void UniClientConn::writeonevalue(const UniConfKey &key, WvStringParm value)
{
    writecmd(REPLY_ONEVAL, spacecat(wvtcl_escape(key), wvtcl_escape(value)));
}


void UniClientConn::writetext(WvStringParm text)
{
    writecmd(PART_TEXT, wvtcl_escape(text));
}
