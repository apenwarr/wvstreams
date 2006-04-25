#include "uniconfroot.h"
#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvdbusserver.h"
#include "wvfile.h"
#include "wvfileutils.h"
#include "wvfork.h"
#include "wvtest.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


static int replies_received = 0;

static void reply_received(WvString foo)
{
    fprintf(stderr, "wow! foo called! (%s)\n", foo.cstr());
    replies_received++;
}

static int messages_received = 0;

static void msg_received(WvDBusReplyMsg &reply, WvString arg1)
{
    fprintf(stderr, "Message received, loud and clear.\n");
    reply.append(WvString("baz %s", arg1));
    messages_received++;
}


#if 0
// FIXME: we can't run this test through valgrind because it complains
// (erroneously, I think) about a leaking message. i think it's just
// d-bus caching the message and not freeing it, but i'm not 100% sure
WVTEST_SLOW_MAIN("basic sanity")
{
    signal(SIGPIPE, SIG_IGN);

    WvString dsockname("/tmp/wvdbus-test-%s-%s", rand(), getpid());
    WvString busfname("/tmp/wvdbus-test-%s-%s", rand(), getpid());
    WvString moniker("unix:tmpdir=%s", dsockname);

    pid_t child = wvfork();
    if (child == 0)
    {
        WvDBusServer server(moniker);
        WvString addr = server.get_addr();
        WvFile(busfname, O_WRONLY|O_CREAT).print("%s\n", addr);

        WvIStreamList::globallist.append(&server, false);
        
        while (true)
            WvIStreamList::globallist.runonce();
	_exit(0);
    }

    WVPASS(child >= 0);

    struct stat st;
    while (stat(busfname, &st) != 0)
        sleep(1);

    WvString addr = WvFile(busfname, O_RDONLY).getline(-1);
    fprintf(stderr, "Address is '%s'\n", addr.cstr());

    WvDBusConn conn1("ca.nit.MySender", addr);
    WvDBusConn conn2("ca.nit.MyListener", addr);
    WvDBusListener<WvString> *l = 
        new WvDBusListener<WvString>(&conn2, "bar", msg_received);
    conn2.add_method("ca.nit.foo", "/ca/nit/foo", l);

    WvDBusMsg msg("ca.nit.MyListener", "/ca/nit/foo", "ca.nit.foo", "bar");
    msg.append("bee");

    WvDBusMarshaller<WvString> reply("/ca/nit/foo/bar",
                                     WvCallback<void, WvString>(reply_received));
    conn1.send(msg, &reply, false);
    WvIStreamList::globallist.append(&conn1, false);
    WvIStreamList::globallist.append(&conn2, false);

    time_t now = time(NULL);

    fprintf(stderr, "Spinning..\n");
    while (replies_received < 1 || messages_received < 1)
         WvIStreamList::globallist.runonce();

    WVPASSEQ(messages_received, 1);
    WVPASSEQ(replies_received, 1);

    conn1.close();
    conn2.close();

    kill(child, 15);
    pid_t rv;
    while ((rv = waitpid(child, NULL, 0)) != child)
    {
        // in case a signal is in the process of being delivered..
        if (rv == -1 && errno != EINTR)
            break;
    }
    WVPASS(rv == child);

    unlink(busfname);
    // see FIXME above. this doesn't help, although maybe it should?
    // note that "msg" above would have to be turned into a pointer and
    // explicitly freed first.
    // dbus_shutdown()
}

#endif
