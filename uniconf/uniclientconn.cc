/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a connection between the UniConf client and daemon.
 */
#include "uniclientconn.h"
#include "wvaddr.h"
#include "wvtclstring.h"
#include "wvistreamlist.h"


/***** UniClientConn *****/

const UniClientConn::CommandInfo UniClientConn::cmdinfos[
    UniClientConn::NUM_COMMANDS] = {
    // requests
    { "noop", "noop: verify that the connection is active" },
    { "get", "get <key>: get the value of a key" },
    { "set", "set <key> <value>: sets the value of a key" },
    { "del", "del <key>: deletes the key" },
    { "subt", "subt <key>: enumerates the children of a key" },
    { "hchild", "hchild <key>: returns whether a key has children" },
    { "quit", "quit: kills the session nicely" },
    { "help", "help: returns this help text" },
    
    // command completion replies
    { "OK", "OK <payload>: reply on command success" },
    { "FAIL", "FAIL <payload>: reply on command failure" },
    { "CHILD", "CHILD <key> TRUE / FALSE: key has children or not" },
    { "ONEVAL", "ONEVAL <key> <value>: reply to a gate" },

    // partial replies
    { "VAL", "VAL <key> <value>: intermediate reply value of a key" },
    { "TEXT", "TEXT <text>: intermediate reply of a text message" },

    // events
    { "HELLO", "HELLO <message>: sent by server on connection" },
    { "NOTICE", "NOTICE <key> <oldval> <newval>: forget key and its children" }
};


UniClientConn::UniClientConn(IWvStream *_s, WvStringParm dst) :
    WvStreamClone(_s),
    log(WvString("UniConf to %s", dst.isnull() ? *_s->src() : WvString(dst)),
    WvLog::Debug5), closed(false), payloadbuf("")
{
    WvIStreamList::globallist.append(this, false);
    log("Opened\n");
}


UniClientConn::~UniClientConn()
{
    WvIStreamList::globallist.unlink(this);
    close();
}


bool UniClientConn::pre_select(SelectInfo &si)
{
    if (si.wants.readable && msgbuf.used()) return true;
    return WvStreamClone::pre_select(si);
}

    
bool UniClientConn::isok() const
{
    return msgbuf.used() != 0 || WvStreamClone::isok();
}


void UniClientConn::close()
{
    if (! closed)
    {
        closed = true;
        WvStreamClone::close();
        log("Closed\n");
    }
}


WvString UniClientConn::readmsg()
{
    WvString word;
    while ((word = wvtcl_getword(msgbuf, "\n", false)).isnull())
    {
        char *line = getline(0);
        if (line)
        {
            msgbuf.putstr(line);
            msgbuf.put('\n');
        }
        else
        {
            if (! isok())
            {
                // possibly left some incomplete command behind
                msgbuf.zap();
            }
            return WvString::null;
        }
    }
    log("Read: %s\n", word);
    return word;
}


void UniClientConn::writemsg(WvStringParm msg)
{
    write(WvString("%s\n", msg));
    log("Wrote: %s\n", msg);
}


UniClientConn::Command UniClientConn::readcmd()
{
    for (;;)
    {
        WvString msg(readmsg());
        if (msg.isnull())
            return NONE;

        // extract command, leaving the remainder in payloadbuf
        payloadbuf.reset(msg);
        WvString cmd(readarg());
        if (cmd.isnull())
            return INVALID;

        for (int i = 0; i < NUM_COMMANDS; ++i)
            if (strcasecmp(cmdinfos[i].name, cmd.cstr()) == 0)
                return Command(i);
        return INVALID;
    }
}


WvString UniClientConn::readarg()
{
    return wvtcl_getword(payloadbuf, " ");
}


void UniClientConn::writecmd(UniClientConn::Command cmd, WvStringParm msg)
{
    if (msg)
        writemsg(WvString("%s %s", cmdinfos[cmd].name, msg));
    else
        writemsg(cmdinfos[cmd].name);
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
    writecmd(PART_VALUE, WvString("%s %s", wvtcl_escape(key),
        wvtcl_escape(value)));
}


void UniClientConn::writeonevalue(const UniConfKey &key, WvStringParm value)
{
    writecmd(REPLY_ONEVAL, WvString("%s %s", wvtcl_escape(key),
        wvtcl_escape(value)));
}


void UniClientConn::writetext(WvStringParm text)
{
    writecmd(PART_TEXT, wvtcl_escape(text));
}
