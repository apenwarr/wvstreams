#include "wvdbusmsg.h"
#include "wvdbusconn.h"
#include "wvdbusserver.h"
#include "wvfileutils.h"
#include "wvfork.h"
#include "wvtest.h"
#include "wvloopback.h"


class TestDBusServer
{
public:
    WvString moniker;
    WvDBusServer *s;
    
    TestDBusServer()
    {
	fprintf(stderr, "Creating a test DBus server.\n");
	WvString smoniker("unix:tmpdir=%s.dir",
			 wvtmpfilename("wvdbus-sock-"));
	s = new WvDBusServer(smoniker);
	moniker = s->get_addr();
	fprintf(stderr, "Server address is '%s'\n", moniker.cstr());
	WvIStreamList::globallist.append(s, false);
    }
    
    ~TestDBusServer()
    {
	delete s;
    }
};


static int mysignal_count = 0;
static bool mysignal(WvDBusMsg &msg)
{
    if (msg.get_interface() != "x.y.z.anything") return false;
    fprintf(stderr, "Got a message! (%s)\n", ((WvString)msg).cstr());
    mysignal_count++;
    return true; // we handle *any* message
}


WVTEST_MAIN("dbusserver basics")
{
    int junk;
    TestDBusServer serv;
    WvDBusConn conn1(serv.moniker);
    WvIStreamList::globallist.append(&conn1, false);
    
    conn1.request_name("ca.nit.MySender");
    
    conn1.add_callback(WvDBusConn::PriNormal, mysignal, &junk);
    WVPASSEQ(mysignal_count, 0);
    
    WvDBusMsg("ca.nit.MySender", "/foo", "x.y.z.anything", "testmethod")
	.append("hello").send(conn1);
    WvDBusSignal("/foo", "x.y.z.anything", "testsignal")
	.append("hello").send(conn1);
    
    while (mysignal_count < 2)
         WvIStreamList::globallist.runonce();
    
    WVPASSEQ(mysignal_count, 2);
}


    
static int replies_received = 0;
static bool reply_received(WvDBusMsg &msg)
{
    WvDBusMsg::Iter i(msg);
    WvString s = i.getnext();
    fprintf(stderr, "wow! reply received! (%s)\n", s.cstr());
    WVPASS(!!msg.get_sender());
    replies_received++;
    return true;
}

    
static int messages_received = 0;
static bool msg_received(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvDBusMsg::Iter i(msg);
    WvString arg1 = i.getnext();
    if (msg.get_dest() == "ca.nit.MyListener"
	&& msg.get_path() == "/ca/nit/foo"
	&& msg.get_member() == "bar")
    {
	fprintf(stderr, "Message received (%s)\n", ((WvString)msg).cstr());
        WVPASS(!!msg.get_sender());
	messages_received++;
	msg.reply().append(WvString("baz %s", arg1)).send(conn);
	return true;
    }
    else
    {
	fprintf(stderr, "msg_received: not my message.\n");
	return false;
    }
}


static int reg_count = 0;
bool name_registered(WvDBusMsg &msg)
{
    reg_count++;
    return true;
}


WVTEST_MAIN("dbusserver two connections")
{
    TestDBusServer serv;
    WvDBusConn conn1(serv.moniker);
    WvDBusConn conn2(serv.moniker);
    WvIStreamList::globallist.append(&conn1, false);
    WvIStreamList::globallist.append(&conn2, false);
    
    conn2.add_callback(WvDBusConn::PriNormal,
		       wv::bind(msg_received, wv::ref(conn2), _1));
    
    reg_count = 0;
    conn1.request_name("ca.nit.MySender", name_registered);
    conn2.request_name("ca.nit.MyListener", name_registered);

    while (reg_count < 2)
         WvIStreamList::globallist.runonce();
    
    WvDBusMsg msg("ca.nit.MyListener", "/ca/nit/foo", "ca.nit.foo", "bar");
    msg.append("bee");

    conn1.send(msg, reply_received);

    while (replies_received < 1 || messages_received < 1)
         WvIStreamList::globallist.runonce();

    WVPASSEQ(messages_received, 1);
    WVPASSEQ(replies_received, 1);
}


WVTEST_MAIN("dbusserver overlapping registrations")
{
    TestDBusServer serv;
    WvDBusConn *cli = new WvDBusConn(serv.moniker);
    WvDBusConn *l1 = new WvDBusConn(serv.moniker);
    WvDBusConn *l2 = new WvDBusConn(serv.moniker);
    WvIStreamList::globallist.append(cli, false);
    WvIStreamList::globallist.append(l1, false);
    WvIStreamList::globallist.append(l2, false);
    
    reg_count = 0;
    l1->request_name("ca.nit.MySender", name_registered);
    while (reg_count < 1)
         WvIStreamList::globallist.runonce();
    
    l1->add_callback(WvDBusConn::PriNormal, mysignal);
    l2->add_callback(WvDBusConn::PriNormal, mysignal);
    
    WvDBusMsg meth("ca.nit.MySender", "/foo", "x.y.z.anything", "testmethod");
    meth.append("methhello");
    WvDBusSignal sig("/foo", "x.y.z.anything", "testsignal");
    sig.append("sighello");
    
    mysignal_count = 0;
    cli->send(sig);
    cli->send(meth, mysignal);
    while (mysignal_count < 3 || WvIStreamList::globallist.select(200))
         WvIStreamList::globallist.runonce();
    WVPASSEQ(mysignal_count, 3); // one method, two signals
    
    mysignal_count = 0;
    delete l1;
    cli->send(sig);
#if NO_DBUS_PENDING_MEMORY_LEAK
    // this seems to cause a memory leak in dbus 0.60, since nobody cleans up
    // the remaining DBusPendingCall objects if a connection closes.  Browsing
    // the source code for later versions, this seems to have been fixed, but
    // I haven't tried it yet.  Anyway, it's not our bug.  The cleanup
    // activity in newer dbus may reveal that we have a bug too, of course.
    cli->send(meth, mysignal);
#endif
    while (mysignal_count < 1 || WvIStreamList::globallist.select(200))
         WvIStreamList::globallist.runonce();
    WVPASSEQ(mysignal_count, 1); // no method receiver, one signal
    
    mysignal_count = 0;
    reg_count = 0;
    l2->request_name("ca.nit.MySender", name_registered);
    while (reg_count < 1)
         WvIStreamList::globallist.runonce();
    
    cli->send(sig);
    cli->send(meth, mysignal);
    while (mysignal_count < 2 || WvIStreamList::globallist.select(200))
         WvIStreamList::globallist.runonce();
    WVPASSEQ(mysignal_count, 2); // one method receiver, one signal
    
    delete cli;
    delete l2;
}
