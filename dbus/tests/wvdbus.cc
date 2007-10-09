/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which sends a simple signal with one argument (int32: 132). 
 * Best used in conjunction with the dbus-sig-listen program.
 * 
 */ 
#include "wvargs.h"
#include "wvdbusconn.h"
#include "wvistreamlist.h"
#include "wvlinkerhack.h"

WV_LINK_TO(WvTCPConn);

static WvStringList paths;

static bool incoming(WvDBusMsg &msg)
{
    WvStringList::Iter i(paths);
    for (i.rewind(); i.next(); )
    {
	if (*i == msg.get_path())
	{
	    fprintf(stderr, "\n * %s\n\n", ((WvString)msg).cstr());
	    return true;
	}
    }
    return false;
}


int main(int argc, char *argv[])
{
    WvArgs args;
    WvString moniker("dbus:session");
    WvStringList names, remaining_args;
    bool sigtest = false, methtest = false, wait = false;
    
    // args.add_optional_arg("COMMANDS", true);
    args.add_option('m', "moniker", "Specify the dbus bus to use",
		    "MONIKER", moniker);
    args.add_option('p', "path", "Listen on <path>",
		    "PATH", paths);
    args.add_option('n', "name", "Register as <name>",
		    "NAME", names);
    args.add_set_bool_option('S', "signaltest", "Send a test signal", sigtest);
    args.add_set_bool_option('M', "methodtest", "Call test method", methtest);
    args.add_set_bool_option('w', "wait", "Wait forever", wait);
    args.process(argc, argv, &remaining_args);

    WvDBusConn conn(moniker);
    WvIStreamList::globallist.append(&conn, false, "wvdbus conn");
    
    conn.add_callback(WvDBusConn::PriNormal, incoming);
    
    WvStringList::Iter i(names);
    for (i.rewind(); i.next(); )
	conn.request_name(*i);

    if (sigtest)
	WvDBusSignal("/ca/nit/foo", "ca.nit.foo", "BarSignal").send(conn);
    
    if (methtest)
    {
	WvDBusMsg("ca.nit.MyListener", "/ca/nit/foo",
		  "ca.nit.foo", "BarMethod")
	    .append("bee").send(conn);
    }
    
    while (conn.isok() && (wait || !conn.isidle()))
        WvIStreamList::globallist.runonce();
    return 0;
}
