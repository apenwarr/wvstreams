#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniretrygen.h"
#include "uniconfdaemon.h"
#include "uniclientgen.h"
#include "uniinigen.h"
#include "wvfork.h"
#include "wvunixsocket.h"
#include "wvfileutils.h"
#include "wvfile.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static int delta_count;
static void callback(const UniConf &, const UniConfKey &)
{
    ++delta_count;
}


WVTEST_MAIN("deltas")
{
    UniConfRoot uniconf;

    signal(SIGPIPE, SIG_IGN);

    WvString sockname = wvtmpfilename("uniclientgen.t-sock");

    pid_t child = wvfork();
    if (child == 0)
    {
        uniconf.mountgen(new UniTempGen());
        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname);
        WvIStreamList::globallist.append(&daemon, false, "uniconf daemon");
        while (true)
        {
            uniconf.setmeint(uniconf.getmeint()+1);
            WvIStreamList::globallist.runonce();
            usleep(1000);
        }
	_exit(0);
    }
    else
    {
	WVPASS(child >= 0);
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
    
    unlink(sockname);
}


static time_t file_time(WvStringParm filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_mtime;
    else
    {
        wverr->print("stat(%s) failed: %s\n", filename,
                strerror(errno));
        return -1;
    }
}


static WvString get_sockname()
{
    int fd;
    WvString sockname = "/tmp/sockXXXXXX";
    if ((fd = mkstemp(sockname.edit())) == (-1))
        return "";    
    close(fd);

    return sockname;
}


WVTEST_MAIN("commit")
{
    UniConfRoot uniconf;
    WvString ini_file("/tmp/wvtest-uniclientgen-commit-%s.ini", getpid());
    unlink(ini_file);
    
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = get_sockname();

    pid_t child = wvfork();
    if (child == 0)
    {
        UniIniGen *ini_gen = new UniIniGen(ini_file);
        fprintf(stderr, "new UniIniGen() = %p\n", (void *)ini_gen);
        uniconf.mountgen(ini_gen);
        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname);
        WvIStreamList::globallist.append(&daemon, false, "uniconf daemon");
        while (true)
        {
            WvIStreamList::globallist.runonce();
        }
	_exit(0);
    }
    else
    {
	WVPASS(child >= 0);
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

        uniconf.setmeint(time(NULL));
        uniconf.commit();
        time_t old_file_time = file_time(ini_file);
        WVPASS(old_file_time != -1);
        WVPASS(old_file_time <= time(NULL));

        // Wait until ini file is old
        while (time(NULL) <= old_file_time)
            sleep(1);

        uniconf.setmeint(time(NULL));
        uniconf.commit();

        time_t new_file_time = file_time(ini_file);
        WVPASS(new_file_time != -1);
        WVPASS(new_file_time > old_file_time);

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

    unlink(ini_file);
}


WVTEST_MAIN("refresh")
{
    UniConfRoot uniconf;
    WvString ini_file("/tmp/wvtest-uniclientgen-refresh-%s.ini", getpid());
    unlink(ini_file);
    
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = get_sockname();

    pid_t child = wvfork();
    if (child == 0)
    {
        UniIniGen *ini_gen = new UniIniGen(ini_file);
        fprintf(stderr, "new UniIniGen() = %p\n", (void *)ini_gen);
        uniconf.mountgen(ini_gen);
        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname);
        WvIStreamList::globallist.append(&daemon, false, "uniconf daemon");
        while (true)
        {
            WvIStreamList::globallist.runonce();
        }
	_exit(0);
    }
    else
    {
	WVPASS(child >= 0);
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

        WVPASS(uniconf.xget("key") != "value");
        WvFile ini_fd(ini_file, O_WRONLY | O_APPEND | O_CREAT);
        ini_fd.print("[]\nkey = value\n");
        ini_fd.close();
        WVPASS(uniconf.refresh());
        WVPASS(uniconf.xget("key") == "value");

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

    unlink(ini_file);
}

