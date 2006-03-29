#include "wvtest.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "unitempgen.h"
#include "unisubtreegen.h"
#include "uniconfdaemon.h"
#include "uniconfgen-sanitytest.h"
#include "wvfileutils.h"
#include "wvfork.h"
#include "uniclientgen.h"
#include "wvunixsocket.h"

#include <sys/wait.h>

WVTEST_MAIN("UniSubtreeGen Sanity Test")
{
    UniSubtreeGen *gen = new UniSubtreeGen(new UniTempGen(), "/");
    UniConfGenSanityTester::sanity_test(gen, "subtree:temp: /");
    WVRELEASE(gen);
}

int callback_count;
void callback(const UniConf &uniconf, const UniConfKey &key)
{
    wverr->print("callback: %s = %s (count now %s)\n",
            uniconf[key].fullkey(), uniconf[key].getme(),
            ++callback_count);
}

int sub_callback_count;
void sub_callback(const UniConf &uniconf, const UniConfKey &key)
{
    wverr->print("sub_callback: %s = %s (count now %s)\n",
            uniconf[key].fullkey(), uniconf[key].getme(),
            ++sub_callback_count);
}

WVTEST_MAIN("callbacks")
{
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = wvtmpfilename("unisubtreegen.t-sock");
    unlink(sockname);

    pid_t child = wvfork();
    if (child == 0)
    {
        UniConfRoot uniconf("temp:");
        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname);
        WvIStreamList::globallist.append(&daemon, false);
        while (true)
            WvIStreamList::globallist.runonce();
	_exit(0);
    }
    WVPASS(child > 0);
    
    UniConfRoot uniconf;
    UniClientGen *client_gen;
    while (true)
    {
        WvUnixConn *unix_conn;
        client_gen = new UniClientGen(
                unix_conn = new WvUnixConn(sockname));
        if (!unix_conn || !unix_conn->isok()
                || !client_gen || !client_gen->isok())
        {
            WVRELEASE(client_gen);
            wvout->print("Failed to connect, retrying...\n");
            sleep(1);
        }
        else break;
    }
    uniconf.mountgen(client_gen); 

    UniConfRoot sub_uniconf;
    UniClientGen *sub_client_gen;
    while (true)
    {
        WvUnixConn *unix_conn;
        sub_client_gen = new UniClientGen(
                unix_conn = new WvUnixConn(sockname));
        if (!unix_conn || !unix_conn->isok()
                || !sub_client_gen || !sub_client_gen->isok())
        {
            WVRELEASE(sub_client_gen);
            wvout->print("Failed to connect, retrying...\n");
            sleep(1);
        }
        else break;
    }
    UniSubtreeGen *subtree = new UniSubtreeGen(sub_client_gen, "sub");
    sub_uniconf.mountgen(subtree);

    UniWatchList watches;
    watches.add(uniconf, callback, true);
    watches.add(sub_uniconf, sub_callback, true);
    
    uniconf.getme();
    sub_uniconf.getme();

    uniconf["sub"].setme("");
    uniconf["another-sub"].setme("");
    uniconf.getme();
    sub_uniconf.getme();

    callback_count = 0;
    sub_callback_count = 0;
    uniconf["sub/foo"].setme("bar");
    uniconf.getme();
    WVPASSEQ(callback_count, 1);
    sub_uniconf.getme();
    WVPASSEQ(sub_callback_count, 1);

    callback_count = 0;
    sub_callback_count = 0;
    uniconf["another-sub/foo"].setme("bar");
    uniconf.getme();
    WVPASSEQ(callback_count, 1);
    sub_uniconf.getme();
    WVPASSEQ(sub_callback_count, 0); // Should not have received callbacks

    // Never, ever kill a pid <= 0, unless you want all your processes dead...
    if (child > 0)
        kill(child, 15);
    pid_t rv;
    while ((rv = waitpid(child, NULL, 0)) != child)
    {
        // in case a signal is in the process of being delivered..
        if (rv == -1 && errno != EINTR)
            break;
    }
    WVPASS(rv == child);

    unlink(sockname);
}
