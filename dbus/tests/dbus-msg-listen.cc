#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvistreamlist.h"

static void msg_received(WvDBusReplyMsg &reply, int arg1)
{
    fprintf(stderr, "Message received, loud and clear.\n");
    reply.append("baz");
}


int main (int argc, char *argv[])
{
    WvDBusConn conn;
    
    // prototype for receiving + replying to messages
    WvDBusListener<int> l(&conn, "/ca/nit/foo", msg_received);
    conn.add_method("ca.nit.foo", &l);

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
