/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Manages a connection between the UniConf client and daemon.
 */
#include "uniconfconn.h"
#include "wvaddr.h"
#include "wvtclstring.h"

/***** UniConfConn *****/

const UniConfConn::CommandInfo UniConfConn::cmdinfos[
    UniConfConn::NUM_COMMANDS] = {
    // requests
    { "noop", "noop: verify that the connection is active" },
    { "get", "get <key>: get the value of a key" },
    { "set", "set <key> <value>: sets the value of a key" },
    { "del", "del <key>: deletes the key" },
    { "zap", "zap <key>: deletes the children of a key" },
    { "subt", "subt <key>: enumerates the children of a key" },
    { "reg", "reg <key> <depth>: registers for change notification" },
    { "ureg", "ureg <key> <depth>: unregisters for change notification" },
    { "quit", "quit: kills the session nicely" },
    { "help", "help: returns this help text" },
    
    // command completion replies
    { "OK", "OK <payload>: reply on command success" },
    { "FAIL", "FAIL <payload>: reply on command failure" },

    // partial replies
    { "VAL", "VAL <key> <value>: intermediate reply value of a key" },
    { "TEXT", "TEXT <text>: intermediate reply of a text message" },

    // events
    { "HELLO", "HELLO <message>: sent by server on connection" },
    { "CHG", "CHG <key> <depth>: event on changed key" }
};


UniConfConn::UniConfConn(IWvStream *_s) :
    WvStreamClone(_s),
    log(WvString("UniConf to %s", *_s->src()), WvLog::Debug5),
    closed(false), payloadbuf("")
{
    log("Opened\n");
}


UniConfConn::~UniConfConn()
{
    close();
}

bool UniConfConn::isok() const
{
    return msgbuf.used() != 0 || WvStreamClone::isok();
}


void UniConfConn::close()
{
    if (! closed)
    {
        closed = true;
        WvStreamClone::close();
        log("Closed\n");
    }
}


WvString UniConfConn::readmsg()
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


void UniConfConn::writemsg(WvStringParm msg)
{
    write(msg);
    write("\n");
    log("Wrote: %s\n", msg);
}


UniConfConn::Command UniConfConn::readcmd()
{
    for (;;)
    {
        WvString msg(readmsg());
        if (msg.isnull())
            return NONE;

        // extract command, leaving the remainder in payloadbuf
        payloadbuf.reset(msg);
        WvString cmd = wvtcl_getword(payloadbuf, " ");

        for (int i = 0; i < NUM_COMMANDS; ++i)
            if (strcmp(cmdinfos[i].name, cmd.cstr()) == 0)
                return Command(i);
        return INVALID;
    }
}


void UniConfConn::writecmd(UniConfConn::Command cmd, WvStringParm msg)
{
    writemsg(WvString("%s %s", cmdinfos[cmd].name, msg));
}


void UniConfConn::writeok(WvStringParm payload)
{
    writecmd(REPLY_OK, payload);
}


void UniConfConn::writefail(WvStringParm payload)
{
    writecmd(REPLY_FAIL, payload);
}


void UniConfConn::writevalue(const UniConfKey &key, WvStringParm value)
{
    writecmd(PART_VALUE, WvString("%s %s", wvtcl_escape(key),
        wvtcl_escape(value)));
}


void UniConfConn::writetext(WvStringParm text)
{
    writecmd(PART_TEXT, wvtcl_escape(text));
}
