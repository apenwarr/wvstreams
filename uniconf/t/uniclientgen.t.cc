#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniretrygen.h"
#include "uniconfdaemon.h"
#include "uniclientgen.h"
#include "wvfork.h"
#include "wvunixsocket.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static WvString get_sockname()
{
    int fd;
    WvString sockname = "/tmp/sockXXXXXX";
    if ((fd = mkstemp(sockname.edit())) == (-1))
        return "";
    close(fd);

    return sockname;
}

static int delta_count;
static void callback(const UniConf &, const UniConfKey &)
{
    ++delta_count;
}

WVTEST_MAIN("deltas")
{
    UniConfRoot uniconf;

    signal(SIGPIPE, SIG_IGN);

    WvString sockname = get_sockname();

    pid_t child = fork();
    WVPASS(child >= 0);
    if (child == 0)
    {
        uniconf.mountgen(new UniTempGen());
        UniConfDaemon daemon(uniconf, false, NULL);
        WVPASS(daemon.setupunixsocket(sockname));
        WvIStreamList::globallist.append(&daemon, false);
        while (true)
        {
            uniconf.setmeint(uniconf.getmeint()+1);
            WvIStreamList::globallist.runonce();
            usleep(1000);
        }
    }
    else
    {
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

        int old_delta_count = delta_count = 0;
        uniconf.add_callback(client_gen, "", callback, false);

        int i;
        for (i=0; i<100; ++i)
        {
            client_gen->refresh();
            usleep(1000);
        }

        WVPASS(delta_count > old_delta_count);
        
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

