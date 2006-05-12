/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which listens for a simple signal with one argument (an
 * int).
 * 
 */ 
#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"


#if 0
static void foo(WvDBusConn &conn, int b)
{
    fprintf(stderr, "wow! foo called! (%i)\n", b);
}
#endif


int main (int argc, char *argv[])
{
#if 0
    WvDBusConn conn("ca.nit.MySignalListener");
    
    WvDBusMarshaller<int> m("bar", WvCallback<void, WvDBusConn&, int>(foo));
    conn.add_marshaller("ca.nit.foo", "/ca/nit/foo", &m);
    WvIStreamList::globallist.append(&conn, false, "wvdbus conn");
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
#endif
    
    return 0;
}
