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
#include "wvistreamlist.h"
#include "wvargs.h"


static bool foo(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvDBusMsg::Iter i(msg);
    int b = i.getnext();
    
    if (msg.get_dest() == "ca.nit.MySignalListener"
	&& msg.get_path() == "/ca/nit/foo"
	&& msg.get_member() == "bar")
    {
	fprintf(stderr, "wow! ca.nit.foo.bar called! (%i)\n", b);
	return true;
    }
    return false;
}


int main(int argc, char *argv[])
{
    WvArgs args;
    WvStringList remaining_args;
    args.add_optional_arg("MONIKER");
    args.process(argc, argv, &remaining_args);
    WvString moniker = remaining_args.popstr();

    WvDBusConn *conn = new WvDBusConn(!!moniker ? moniker : "bus:session");
    WvIStreamList::globallist.append(conn, false, "wvdbus conn");
    
    conn->request_name("ca.nit.MySignalListener");
    conn->add_callback(WvDBusConn::PriNormal, foo);
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    WVRELEASE(conn);
    return 0;
}
