#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"


static void foo(WvString foo)
{
    fprintf(stderr, "wow! foo called! (%s)\n", foo.cstr());
}


int main (int argc, char *argv[])
{
    WvDBusConn conn;
    
//     WvDBusMarshaller<int> m("/ca/nit/foo/bar", WvCallback<void, int>(foo));
//     conn.add_marshaller("ca.nit.foo", &m);

    // prototype for sending messages..
    WvDBusMsg msg("ca.nit.foo", "/ca/nit/foo", "ca.nit.foo", "bar");
    msg.append("bee");
    WvDBusMarshaller<WvString> reply("/ca/nit/foo/bar", WvCallback<void, WvString>(foo));
    conn.send(msg, &reply, false);

//     conn.setcallback(myfilter);
//     conn.request_name("ca.nit.dbustest");

    WvIStreamList::globallist.append(&conn, false);
    
//     WvDBus::CallMsg msg("ca.nit.dbustest", "/ca/nit/dbustest",
// 			"ca.nit.dbustest", "StupidMember");
//     msg.append("yes", "hello", "world");
    
//     {
// 	WvDBus::Msg reply(conn.sendsync(msg, 1000));
// 	wvcon->print("reply: %s\n", (int)(DBusMessage*)reply);
//     }
    
    while (WvIStreamList::globallist.isok())
        WvIStreamList::globallist.runonce();
    
    return 0;


}
