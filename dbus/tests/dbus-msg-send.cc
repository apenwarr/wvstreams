/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which sends a simple message with one argument (string: 
 * "bee"), and waits for a reply, also with a single string as payload. 
 * Best used in conjunction with the dbus-msg-listen program.
 * 
 */ 
#include "wvargs.h"
#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"


static void foo(WvString foo)
{
    fprintf(stderr, "wow! foo called! (%s)\n", foo.cstr());
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
        conn = new WvDBusConn("ca.nit.MySender", moniker);
    else
        conn = new WvDBusConn("ca.nit.MySender");

    // Create a message, bound for "ca.nit.MyApplication"'s "/ca/nit/foo" 
    // object, with the "ca.nit.foo" interface's "bar" method.
    WvDBusMsg msg("ca.nit.MyListener", "/ca/nit/foo", "ca.nit.foo", "bar");
    msg.append("bee");

    // expect a reply with a single string as an argument
    WvDBusListener<WvString> reply("/ca/nit/foo/bar", foo);
    fprintf(stderr, "Sending message..?\n");
    conn->send(msg, &reply, false);

    WvIStreamList::globallist.append(conn, true, "wvdbus conn");
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    return 0;
}
