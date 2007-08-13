#include "wvdbusmsg.h"
#include "wvdbusconn.h"
#include "wvdbusserver.h"
#include "wvfileutils.h"
#include "wvfork.h"
#include "wvtest.h"
#include "wvloopback.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


class TestDBusServer
{
public:
    WvString moniker;
    pid_t parent, child;
    WvLoopback loop;
    
    TestDBusServer()
    {
	signal(SIGPIPE, SIG_IGN);
	fprintf(stderr, "Creating a test DBus server.\n");
	
	parent = getpid();
	child = wvfork(loop.getrfd(), loop.getwfd());
	if (child == 0)
	    do_child(); // never returns
	WVPASS(child >= 0);
	
	moniker = loop.getline(-1);
	fprintf(stderr, "Server address is '%s'\n", moniker.cstr());
    }
    
    ~TestDBusServer()
    {
	fprintf(stderr, "Killing test server.\n");
	kill(child, 15);
	pid_t rv;
	while ((rv = waitpid(child, NULL, 0)) != child)
	{
	    // in case a signal is in the process of being delivered..
	    if (rv == -1 && errno != EINTR)
		break;
	}
	WVPASS(rv == child);
    }
    
    void do_child()
    {
	WvString smoniker("unix:tmpdir=%s.dir",
			 wvtmpfilename("wvdbus-sock-"));
	WvDBusServer server(smoniker);
	
	loop.print("%s\n", server.get_addr());
	
	WvIStreamList::globallist.append(&server, false);
	while (server.isok())
	{
	    WvIStreamList::globallist.runonce(1000);
	    if (kill(parent, 0) < 0) break;
	}
	fprintf(stderr, "Server process terminating.\n");
	_exit(0);
    }
};


static int mysignal_count = 0;
static bool mysignal(WvDBusConn &conn, WvDBusMsg &msg)
{
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
static bool reply_received(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvDBusMsg::Iter i(msg);
    WvString s = i.getnext();
    fprintf(stderr, "wow! reply received! (%s)\n", s.cstr());
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


WVTEST_MAIN("dbusserver two connections")
{
    TestDBusServer serv;
    WvDBusConn conn1(serv.moniker);
    WvDBusConn conn2(serv.moniker);
    WvIStreamList::globallist.append(&conn1, false);
    WvIStreamList::globallist.append(&conn2, false);
    
    conn1.request_name("ca.nit.MySender");
    conn2.request_name("ca.nit.MyListener");
    conn2.add_callback(WvDBusConn::PriNormal, msg_received);

    WvDBusMsg msg("ca.nit.MyListener", "/ca/nit/foo", "ca.nit.foo", "bar");
    msg.append("bee");

    conn1.send(msg, reply_received);

    fprintf(stderr, "Spinning...\n");
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
    
    l1->request_name("ca.nit.MySender");
    
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
    cli->send(meth, mysignal);
    while (mysignal_count < 1 || WvIStreamList::globallist.select(200))
         WvIStreamList::globallist.runonce();
    WVPASSEQ(mysignal_count, 1); // no method receiver, one signal
    
    mysignal_count = 0;
    l2->request_name("ca.nit.MySender");
    cli->send(sig);
    cli->send(meth, mysignal);
    while (mysignal_count < 2 || WvIStreamList::globallist.select(200))
         WvIStreamList::globallist.runonce();
    WVPASSEQ(mysignal_count, 2); // one method receiver, one signal
    
    delete cli;
    delete l2;
}
