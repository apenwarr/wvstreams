#include "wvdbusconn.h"
#include "wvdbusmarshaller.h"
#include "wvdbusserver.h"
#include "wvfileutils.h"
#include "wvfork.h"
#include "wvtest.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// FIXME: broken
#if 0
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

WVTEST_MAIN("basic sanity")
{
    signal(SIGPIPE, SIG_IGN);

    WvString sockname("/tmp/wvdbus-test-%s", getpid());
    WvString moniker("unix:path=%s", sockname);

    pid_t child = wvfork();
    if (child == 0)
    {
        WvDBusServer server(moniker);

        WvIStreamList::globallist.append(&server, false);
        
        while (true)
            WvIStreamList::globallist.runonce();
	_exit(0);
    }
    else
    {
	WVPASS(child >= 0);
        sleep(20);

        WvDBusConn conn1("ca.nit.MySender", moniker);
        WvDBusConn conn2("ca.nit.MyListener", moniker);
        WvDBusListener<WvString> l(&conn2, "bar", msg_received);
        conn2.add_method("ca.nit.foo", "/ca/nit/foo", &l);

        WvDBusMsg msg("ca.nit.MyListener", "/ca/nit/foo", "ca.nit.foo", "bar");
        msg.append("bee");

        WvDBusMarshaller<WvString> reply("/ca/nit/foo/bar",
                                         WvCallback<void, WvString>(reply_received));
        conn1.send(msg, &reply, false);

        WvIStreamList::globallist.append(&conn1, false);
        WvIStreamList::globallist.append(&conn2, false);

        while (replies_received < 1 && messages_received < 1)
            WvIStreamList::globallist.runonce();

        WVPASSEQ(messages_received, 1);
        WVPASSEQ(replies_received, 1);

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
}
#endif
