#include "wvtest.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "unitempgen.h"
#include "unisubtreegen.h"
#include "uniconfdaemon.h"
#include "uniconfgen-sanitytest.h"
#include "wvfileutils.h"
#include "uniclientgen.h"
#include "wvunixsocket.h"

#include <signal.h>

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

    UniConfTestDaemon daemon(sockname, "temp:");
    
    UniConfRoot uniconf;
    
    printf("Waiting for daemon to start.\n");
    fflush(stdout);
    int num_tries = 0;
    const int max_tries = 20;
    while (!uniconf.isok() && num_tries++ < max_tries)
    {
        WVFAIL(uniconf.isok());

        // Try again...
        uniconf.unmount(uniconf.whichmount(), true);
        uniconf.mount(WvString("unix:%s", sockname));
        sleep(1);
    }

    num_tries = 0;
    UniConfRoot sub_uniconf;
    UniClientGen *sub_client_gen;
    while (num_tries++ < max_tries)
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

    unlink(sockname);
}
