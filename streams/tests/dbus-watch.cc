#include "wvdbusconn.h"
#include "wvistreamlist.h"

#if 0
static void myfilter(WvDBus &conn, const WvDBus::Msg &msg)
{
    bool match = 
	dbus_message_is_method_call(msg, "ca.nit.dbustest", "StupidMember");
    wvcon->print("filter_func: %s (matches=%s)\n    ",
		 (int)(void *)msg, match);
    
    WvStringList l;
    msg.decode(l);
    WvStringList::Iter i(l);
    for (i.rewind(); i.next(); )
	wvcon->print("'%s' ", *i);
    wvcon->print("\n");
    
    if (match)
    {
	WvDBus::ReplyMsg reply(msg);
	dbus_message_append_args(reply,
				 DBUS_TYPE_DOUBLE, 42.6,
				 DBUS_TYPE_STRING, "rstring1",
				 DBUS_TYPE_STRING, "rstring2",
				 DBUS_TYPE_INVALID);
	conn.send(reply);
	wvcon->print("sent reply.\n");
    }
}
#endif

int main (int argc, char *argv[])
{
    WvDBusConn conn;
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
