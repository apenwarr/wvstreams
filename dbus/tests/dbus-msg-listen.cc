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
#include "wvdbuslistener.h"
#include "wvistreamlist.h"


static void msg_received(WvDBusConn &conn, WvDBusMsg &msg,
			 WvString arg1, WvError err)
{
    if (err.isok())
    {
        fprintf(stderr, "Message received, loud and clear.\n");
        msg.reply().append(WvString("baz %s", arg1)).send(conn);
    }
    else
        fprintf(stderr, "Received a message, but there was an error (%s).\n",
                err.errstr().cstr());
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
    conn->request_name("ca.nit.MyListener");
    
    // Create a "/ca/nit/foo" object to listen on, and add the "bar"
    // method (part of the 31337 "ca.nit.foo" interface) to it..
    WvDBusListener<WvString> l(conn, "bar", msg_received);
    conn->add_method("ca.nit.foo", "/ca/nit/foo", &l);
    WvIStreamList::globallist.append(conn, false, "wvdbus conn");
    
    while (conn->isok())
        WvIStreamList::globallist.runonce();
    
    WVRELEASE(conn);
    return 0;
}
