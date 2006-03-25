/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Test program which listens for a simple message with one argument (a 
 * string) and sends a reply, a single string. Best used in conjunction with 
 * the dbus-send program.
 * 
 */ 
#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"


static void msg_received(WvDBusReplyMsg &reply, WvString arg1)
{
    fprintf(stderr, "Message received, loud and clear.\n");
    reply.append(WvString("baz %s", arg1));
}


int main (int argc, char *argv[])
{
    WvDBusConn conn("ca.nit.MyListener");
    
    // Create a "/ca/nit/foo" object to listen on, and add the "bar"
    // method (part of the 31337 "ca.nit.foo" interface) to it..
    WvDBusListener<WvString> l(&conn, "bar", msg_received);
    conn.add_method("ca.nit.foo", "/ca/nit/foo", &l);
    WvIStreamList::globallist.append(&conn, false);
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    return 0;
}
