/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which listens for a simple message with one argument (a 
 * string) and sends a reply, a single string. Best used in conjunction with 
 * the dbus-msg-send program.
 * 
 */ 
#include "wvargs.h"
#include "wvdbusconn.h"
#include "wvistreamlist.h"


static bool msg_received(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvDBusMsg::Iter i(msg);
    WvString arg1 = i.getnext();
    
    if (msg.get_dest() == "ca.nit.MyListener"
	&& msg.get_path() == "/ca/nit/foo"
	&& msg.get_member() == "bar")
    {
        fprintf(stderr, "Message received:\n  %s\n", ((WvString)msg).cstr());
        msg.reply().append(WvString("baz %s", arg1)).send(conn);
	return true;
    }
    return false;
}


int main (int argc, char *argv[])
{
    WvArgs args;
    WvStringList remaining_args;
    args.add_optional_arg("MONIKER");
    args.process(argc, argv, &remaining_args);
    WvString moniker = remaining_args.popstr();

    WvDBusConn *conn;
    if (!!moniker)
        conn = new WvDBusConn(moniker);
    else
        conn = new WvDBusConn();
    WvIStreamList::globallist.append(conn, false, "wvdbus conn");
    
    conn->request_name("ca.nit.MyListener");
    conn->add_callback(WvDBusConn::PriNormal, msg_received);
    
    while (conn->isok())
        WvIStreamList::globallist.runonce();
    
    WVRELEASE(conn);
    return 0;
}
