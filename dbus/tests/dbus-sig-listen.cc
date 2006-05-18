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
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"


static void foo(int b)
{
    fprintf(stderr, "wow! foo called! (%i)\n", b);
}


int main (int argc, char *argv[])
{
    WvDBusConn conn("ca.nit.MySignalListener");
    
    WvDBusSignalListener<int> m("bar", &foo);
    conn.add_listener("ca.nit.foo", "/ca/nit/foo", &m);
    WvIStreamList::globallist.append(&conn, false, "wvdbus conn");
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    return 0;
}
