/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which listens for a simple signal with one argument (an
 * int).
 * 
 * To get it receiving a signal, try running dbus-send as follows:
 *   'dbus-send /ca/nit/foo ca.nit.foo.bar int32:12'
 */ 
#include "wvdbusconn.h"
#include "wvdbuslistener.h"
#include "wvistreamlist.h"
#include "wvargs.h"


static void foo(WvDBusConn &conn, WvDBusMsg &msg, int b, WvError err)
{
    if (err.isok())
        fprintf(stderr, "wow! ca.nit.foo.bar called! (%i)\n", b);
    else
        fprintf(stderr, "oops, our signal was called, but there was a "
                "problem! (%s)", err.errstr().cstr());
}


int main(int argc, char *argv[])
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
    conn->request_name("ca.nit.MySignalListener");
    
    WvDBusListener<int> m(conn, "bar", &foo);
    conn->add_listener("ca.nit.foo", "/ca/nit/foo", &m);

    WvIStreamList::globallist.append(conn, false, "wvdbus conn");
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    return 0;
}
