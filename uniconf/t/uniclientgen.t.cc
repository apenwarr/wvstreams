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
#include "uniwatch.h"
#include "wvstrutils.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static int delta_count;
static void callback(const UniConf &, const UniConfKey &)
{
    ++delta_count;
}

void boring_server_cb(WvStringParm sockname, WvStringParm server_moniker)
{
    wverr->close();
    UniConfRoot uniconf(server_moniker);
    UniConfDaemon daemon(uniconf, false, NULL);
    daemon.setupunixsocket(sockname);
    WvIStreamList::globallist.append(&daemon, false);
    while (true)
        WvIStreamList::globallist.runonce();
    _exit(0);
}

void autoincrement_server_cb(WvStringParm sockname, WvStringParm server_moniker)
{
    wverr->close();
    UniConfRoot uniconf(server_moniker);
    UniConfDaemon daemon(uniconf, false, NULL);
    daemon.setupunixsocket(sockname);
    WvIStreamList::globallist.append(&daemon, false);
    while (true)
    {
        uniconf.setmeint(uniconf.getmeint()+1);
        WvIStreamList::globallist.runonce();
        usleep(1000);
    }
    _exit(0);
}

// server_cb is the main body of the server.  It must not return.
static pid_t setup_server(WvStringParm sockname, WvStringParm server_moniker, 
    WvCallback<void, WvStringParm, WvStringParm> server_cb = boring_server_cb)
{
    pid_t child = wvfork();
    if (child == 0)
        server_cb(sockname, server_moniker);
    WVPASS(child > 0);

    return child;
}


static void cleanup_server(pid_t pid, WvStringParm sockname)
{
    kill(pid, 15);
    pid_t rv;
    while ((rv = waitpid(pid, NULL, 0)) != pid)
    {
        // in case a signal is in the process of being delivered..
        if (rv == -1 && errno != EINTR)
            break;
    }
    WVPASS(rv == pid);
    
    unlink(sockname);
}


WVTEST_MAIN("deltas")
{
    UniConfRoot uniconf;

    signal(SIGPIPE, SIG_IGN);

    WvString sockname = wvtmpfilename("uniclientgen.t-sock");
    unlink(sockname);

    pid_t server_pid = setup_server(sockname, "temp:", autoincrement_server_cb);
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
    
    cleanup_server(server_pid, sockname);
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

    pid_t server_pid = setup_server(sockname, WvString("ini:%s", ini_file));

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

    cleanup_server(server_pid, sockname);
    unlink(ini_file);
}


WVTEST_MAIN("refresh")
{
    UniConfRoot uniconf;
    WvString ini_file("/tmp/wvtest-uniclientgen-refresh-%s.ini", getpid());
    unlink(ini_file);
    
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = get_sockname();

    pid_t server_pid = setup_server(sockname, WvString("ini:%s", ini_file));

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

    cleanup_server(server_pid, sockname);

    unlink(ini_file);
}

int save_callback_calls;
void save_callback()
{
    ++save_callback_calls;
}

WVTEST_MAIN("SaveCallback")
{
    UniConfRoot uniconf;

    WvString ini_file("/tmp/wvtest-uniclientgen-refresh-%s.ini", getpid());
    unlink(ini_file);

    UniIniGen *ini_gen = new UniIniGen(ini_file, 0600, save_callback);
    uniconf.mountgen(ini_gen);
    
    int items = 0;
    uniconf["Items"].xsetint(items, items++);
    uniconf["Items"].xsetint(items, items++);
    uniconf["Items"].xsetint(items, items++);
    uniconf["Items"].xsetint(items, items++);
    uniconf["Items"].xsetint(items, items++);

    save_callback_calls = 0;
    uniconf.commit();
    WVPASS(save_callback_calls >= items);
    
    unlink(ini_file);
}

class WvDebugUnixConn : public WvUnixConn
{
    WvLog log;
    WvString socket;
public:
    WvDebugUnixConn(WvString _name, WvStringParm _socket) :
        WvUnixConn(_socket),
        log(_name, WvLog::Debug),
        socket(_socket)
    {
        log("Created UNIX connection on %s\n", socket);
    }

    ~WvDebugUnixConn()
    {
        log("Destroyed UNIX connection on %s\n", socket);
    }

    size_t uread(void *buf, size_t count)
    {
        size_t result = WvUnixConn::uread(buf, count);
        if (result > 0)
            log("<- %s\n", cstr_escape(buf, result));
        return result;
    }

    size_t uwrite(const void *buf, size_t count)
    {
        size_t result = WvUnixConn::uwrite(buf, count);
        if (result > 0)
            log("-> %s\n", cstr_escape(buf, result));
        return result;
    }
};
        
int restrict_callback_count;
void restrict_callback(const UniConf &uniconf, const UniConfKey &key)
{
    wverr->print("restrict_callback: %s = %s (count now %s)\n",
            uniconf[key].fullkey(), uniconf[key].getme(),
            ++restrict_callback_count);
}

int sub_restrict_callback_count;
void sub_restrict_callback(const UniConf &uniconf, const UniConfKey &key)
{
    wverr->print("sub_restrict_callback: %s = %s (count now %s)\n",
            uniconf[key].fullkey(), uniconf[key].getme(),
            ++sub_restrict_callback_count);
}

WVTEST_MAIN("restrict")
{
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = wvtmpfilename("unisubtreegen.t-sock");
    unlink(sockname);

    pid_t server_pid = setup_server(sockname, "temp:");
    
    UniConfRoot uniconf;
    UniClientGen *client_gen;
    while (true)
    {
        WvUnixConn *unix_conn;
        client_gen = new UniClientGen(
                unix_conn = new WvDebugUnixConn("top", sockname));
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
                unix_conn = new WvDebugUnixConn("sub", sockname), sockname,
                "sub");
        if (!unix_conn || !unix_conn->isok()
                || !sub_client_gen || !sub_client_gen->isok())
        {
            WVRELEASE(sub_client_gen);
            wvout->print("Failed to connect, retrying...\n");
            sleep(1);
        }
        else break;
    }
    sub_uniconf.mountgen(sub_client_gen);

    UniWatchList watches;
    watches.add(uniconf, restrict_callback, true);
    watches.add(sub_uniconf, sub_restrict_callback, true);
    
    uniconf.getme();
    sub_uniconf.getme();

    uniconf["sub"].setme("");
    uniconf["another-sub"].setme("");
    uniconf.getme();
    sub_uniconf.getme();

    restrict_callback_count = 0;
    sub_restrict_callback_count = 0;
    uniconf["sub/foo"].setme("bar");
    uniconf.getme();
    WVPASSEQ(restrict_callback_count, 1);
    sub_uniconf.getme();
    WVPASSEQ(sub_restrict_callback_count, 1);

    restrict_callback_count = 0;
    sub_restrict_callback_count = 0;
    uniconf["another-sub/foo"].setme("bar");
    uniconf.getme();
    WVPASSEQ(restrict_callback_count, 1);
    sub_uniconf.getme();
    WVPASSEQ(sub_restrict_callback_count, 0); // Should not have received callbacks

    cleanup_server(server_pid, sockname);
}


WVTEST_MAIN("timeout")
{
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = wvtmpfilename("unisubtreegen.t-sock");
    unlink(sockname);

    pid_t server_pid = setup_server(sockname, "temp:");
    
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

    client_gen->set_timeout(1000);

    uniconf.getme();
    WVPASS(client_gen->isok());
    kill(server_pid, SIGSTOP);
    uniconf.getme();
    WVPASS(!client_gen->isok());

    kill(server_pid, SIGCONT);
    cleanup_server(server_pid, sockname);
}

#if 0
// Can't run this test -- it takes at least 60 seconds...
WVTEST_MAIN("dead uniconfd")
{
    UniConfRoot uniconf;
    
    signal(SIGPIPE, SIG_IGN);

    WvString sockname = get_sockname();

    // FIXME: Use setup_server()
    pid_t child = wvfork();
    if (child == 0)
    {
        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname);
        WvIStreamList::globallist.append(&daemon, false);
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

        kill(child, SIGSTOP);
        uniconf.getme();
        WVFAIL(client_gen->isok());

    }
    cleanup_server(child, sockname);
}
#endif
