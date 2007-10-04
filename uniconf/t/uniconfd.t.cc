#include "wvtest.h"
#include "uniclientconn.h"
#include "uniconfdaemon.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "wvstringlist.h"
#include "wvistreamlist.h"
#include "wvunixsocket.h"
#include "wvstreamclone.h"
#include "wvlog.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvfile.h"
#include "wvpipe.h"
#include "wvstringlist.h"
#include "wvfileutils.h"
#include <signal.h>

/**** Generic daemon testing helpers ****/

DeclareWvList(WvStringList);

class UniConfDaemonTestConn : public WvStreamClone
{
public:
    UniConfDaemonTestConn(IWvStream *s, WvStringList *_commands, 
                            WvStringListList *_expected_responses) :        
        WvStreamClone(s),
        commands(_commands),
        expected_responses(_expected_responses),
        log("UniConfDaemonTestConn", WvLog::Debug)      
        {
            setclone(s);
            uses_continue_select = true;
        }
    virtual ~UniConfDaemonTestConn() 
        {
            log("Destructing\n");
            terminate_continue_select();            
        }

    virtual void close()
        {
            log("Closing connection\n");
	    WvStreamClone::close();
        }

    virtual void execute() 
        {
            WvStreamClone::execute();
            
            // order is always process responses to last command (or conn),
            // then send the next command
            // if we're out of expected responses, that means we're done
            if (expected_responses->count())
            {
                while (expected_responses->first()->count())
                {
                    WvString line = blocking_getline(-1);
                    WvString expected = expected_responses->first()->popstr();
                    log(">> '%s' (expect: '%s')\n", line, expected);
                    WVPASS(!strcmp(line.cstr(), expected.cstr()));
                } 
                expected_responses->unlink_first();
                
                if (commands->count() > 0)
                {
                    WvString command = commands->popstr();
                    print("%s\n", command);
                    log("<< %s\n", command);
                }
            }

            // expecting nothing more, can close
            if (!expected_responses->count())
            {
		log("no more responses expected, closing\n");
                close();
		if (isok())
		    log("um, why are we still ok?\n");
            }
        }

private:
    WvStringList *commands;
    WvStringListList *expected_responses;
    WvLog log;
};


/**** Daemon surprise close test and helpers ****/
#if 0
static void spin(WvIStreamList &l)
{   
    int max;
    for (max = 0; max < 100 && l.select(10); max++)
    {
	wvcon->print(".");
	l.callback();
    }
    wvcon->print("\n");
    WVPASS(max < 100);
    
}


static void appendbuf(WvStream &s, void *_buf)
{
    printf("append!\n");
    WvDynBuf *buf = (WvDynBuf *)_buf;
    s.read(*buf, 10240);
}


static void linecmp(WvIStreamList &sl, WvBuf &buf,
		    const char *w1, const char *w2 = NULL,
		    const char *w3 = NULL)
{
    printf("%s", WvString("Awaiting '%s' '%s' '%s'\n", w1, w2, w3).cstr());
    spin(sl);
    
    WvString line = wvtcl_getword(buf, "\r\n");
    WVPASS(line);
    if (!line) return;
    
    WvStringList l;
    wvtcl_decode(l, line);
    
    size_t nargs = w3 ? 3 : (w2 ? 2 : 1);
    WVPASS(l.count() < 4);
    
    WVPASSEQ(l.popstr(), w1);
    if (nargs >= 2) WVPASSEQ(l.popstr(), w2);
    if (nargs >= 3) WVPASSEQ(l.popstr(), w3);
}


WVTEST_MAIN("daemon surprise close")
{
    WvIStreamList l;
    WVPASSEQ(WvIStreamList::globallist.count(), 0);
    spin(WvIStreamList::globallist);
    
    signal(SIGPIPE, SIG_IGN);
    WvIPPortAddr addr("0.0.0.0:4113");
    
    UniConfRoot cfg("temp:");
    UniConfDaemon daemon(cfg, false, NULL);
    
    spin(l);
    
    WVPASS(daemon.isok());
    daemon.setuptcpsocket(addr);
    WVPASS(daemon.isok());
    
    spin(l);
    
    WvDynBuf buf;
    WvTCPConn tcp(addr);
    tcp.setcallback(appendbuf, &buf);
    WVPASS(tcp.isok());
    
    l.append(&daemon, false);
    l.append(&tcp, false);
    
    linecmp(l, buf, "HELLO");
    WVPASS(tcp.isok());

    tcp.write("SET /x/y z\n");
    tcp.close();
    spin(l);
    //linecmp(l, buf, "NOTICE", "", "");
    //linecmp(l, buf, "NOTICE", "x", "");
    //linecmp(l, buf, "NOTICE", "x/y", "z");
}
#endif

/**** Daemon multimount test ****/


WVTEST_MAIN("daemon multimount")
{
    signal(SIGPIPE, SIG_IGN);

    UniConfRoot cfg("temp:");

    cfg["pickles"].setme("foo");
    cfg["subtree/fries"].setme("bar1");
    cfg["subtree/ketchup"].setme("bar2");
    cfg["subt"].mount("temp:");
    cfg["subt/mayo"].setme("baz");

    UniConfDaemon daemon(cfg, false, NULL);

    WvStringList commands;
    commands.append("subt / 1");
    WvStringListList expected_responses;
    WvStringList hello_response;
    hello_response.append(WvString("HELLO {UniConf Server ready.} %s",
				   UNICONF_PROTOCOL_VERSION));
    expected_responses.add(&hello_response, false);
    WvStringList expected_quit_response;
    expected_quit_response.append("VAL subtree {}");
    expected_quit_response.append("VAL subtree/fries bar1");
    expected_quit_response.append("VAL subtree/ketchup bar2");
    expected_quit_response.append("VAL subt {}");
    expected_quit_response.append("VAL subt/mayo baz");
    expected_quit_response.append("VAL pickles foo");
    expected_quit_response.append("OK ");
    expected_responses.add(&expected_quit_response, false);

    WvString pipename = wvtmpfilename("uniconfd.t-pipe");
    WVPASS(daemon.setupunixsocket(pipename));
    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonTestConn conn(sock, &commands, &expected_responses);

    WvIStreamList::globallist.append(&conn, false, "connection");
    WvIStreamList::globallist.append(&daemon, false, "daemon");
    printf("You are about to enter the no spin zone\n");
    while (!WvIStreamList::globallist.isempty() && 
           conn.isok() && daemon.isok())
    {
        printf("Spinning: streams left: %i\n", 
               WvIStreamList::globallist.count());
        WvIStreamList::globallist.runonce();
    }

    WVPASS(daemon.isok());
    WvIStreamList::globallist.zap();
    fprintf(stderr, "we're here\n");
}


/**** Daemon quit test ****/

// sort of useless: this functionality already exists in
// daemon/tests and is automatically exercised. furthermore, we only
// test that the server responds with 'OK' upon close (rather than
// actually testing whether or not it closed). leaving this
// here as a simple (generic) example to write future tests
WVTEST_MAIN("daemon quit")
{
    UniConfRoot cfg("temp:");
    signal(SIGPIPE, SIG_IGN);

    cfg["pickles"].setme("foo");
    cfg["subtree/fries"].setme("bar1");
    cfg["subtree/ketchup"].setme("bar2");
    UniConfDaemon daemon(cfg, false, NULL);

    WvStringList commands;
    commands.append("quit");
    WvStringListList expected_responses;
    WvStringList hello_response;
    hello_response.append(WvString("HELLO {UniConf Server ready.} %s",
				   UNICONF_PROTOCOL_VERSION));
    expected_responses.add(&hello_response, false);
    WvStringList expected_quit_response;
    expected_quit_response.append("OK ");
    expected_responses.add(&expected_quit_response, false);

    WvString pipename = wvtmpfilename("uniconfd.t-pipe");
    WVPASS(daemon.setupunixsocket(pipename));
    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonTestConn conn(sock, &commands, &expected_responses);

    WvIStreamList::globallist.append(&conn, false, "conn");
    WvIStreamList::globallist.append(&daemon, false, "daemon");
    printf("You are about to enter the no spin zone\n");
    while (!WvIStreamList::globallist.isempty() && 
           conn.isok() && daemon.isok())
    {
        printf("Spinning: streams left: %i\n", 
               WvIStreamList::globallist.count());
        WvIStreamList::globallist.runonce();
    }

    WVPASS(daemon.isok());
    WvIStreamList::globallist.zap();
}


/**** Daemon proxying test ****/

// test that proxying between two uniconf daemons works
// e.g.: client -> uniconfd -> uniconfd

static WvPipe * setup_master_daemon(bool implicit_root, 
        WvString &masterpipename, WvString &ininame)
{
    ininame = wvtmpfilename("uniconfd.t-ini");
    WvString pidfile = wvtmpfilename("uiconfd.t-mpid");

    WvFile stuff(ininame, (O_CREAT | O_WRONLY));
    stuff.print("pickles/apples/foo=1\n");
    stuff.print("pickles/mangos/bar=1\n");
    stuff.close();

    masterpipename = wvtmpfilename("uniconfd.t-sock");

    WvString inimount("/cfg=ini:%s", ininame);    
    WvString mount1, mount2;
    if (implicit_root)
        mount1 = inimount;
    else
    {
        mount1 = "/=temp:";
        mount2 = inimount;
    }

    const char * const uniconfd_args[] = {
        "uniconf/daemon/uniconfd",
        "-d", "--pid-file", pidfile.cstr(),
        "-p", "0",
        "-s", "0",
        "-u", masterpipename.cstr(),
        mount1.cstr(),
        mount2.cstr(),
        NULL
    };

    return new WvPipe(uniconfd_args[0], uniconfd_args, false, true, false);
}


static WvPipe * setup_slave_daemon(bool implicit_root, WvStringParm masterpipename, 
                            WvString &slavepipename)
{
    slavepipename = wvtmpfilename("uniconfd.t-sock");
    WvString pidfile = wvtmpfilename("uiconfd.t-spid");

    WvString rootmount;
    if (implicit_root)
        rootmount.append("/=retry:unix:%s", masterpipename);
    else
        rootmount.append("/=retry:cache:unix:%s", masterpipename);

    const char * const uniconfd_args[] = {
        "uniconf/daemon/uniconfd",
        "-d", "--pid-file", pidfile.cstr(),
	"-p", "0",
        "-s"  "0",
        "-u", slavepipename.cstr(),
        rootmount.cstr(),
        NULL
    };

    return new WvPipe(uniconfd_args[0], uniconfd_args, false, false, false); 
}


static void wait_for_pipe_ready(WvStringParm pipename)
{
    // If we can't get a connection in 100ms, something is seriously wrong..

    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);

    WvString line;
    line = sock->getline(100);
    while (!line)
    {
        WVRELEASE(sock);
        sock = new WvUnixConn(addr);
        line = sock->getline(100);
    }
    WVRELEASE(sock);
    fprintf(stderr, "Pipe ready! (%s)\n", line.cstr());
}


static void daemon_proxy_test(bool implicit_root)
{
    WvString masterpipename, ininame;
    WvPipe *master = setup_master_daemon(implicit_root, masterpipename, ininame);
    master->setcallback(wv::bind(WvPipe::ignore_read, wv::ref(*master)));
    master->nowrite();
    wait_for_pipe_ready(masterpipename);
    
    WvString slavepipename;
    WvPipe *slave = setup_slave_daemon(implicit_root, masterpipename, slavepipename);
    slave->setcallback(wv::bind(WvPipe::ignore_read, wv::ref(*slave)));
    slave->nowrite();
    wait_for_pipe_ready(slavepipename);

    WvStringList commands;
    commands.append("get /cfg/pickles/apples/foo");
    commands.append("subt /");
    WvStringListList expected_responses;
    WvStringList hello_response;
    hello_response.append(WvString("HELLO {UniConf Server ready.} %s",
				   UNICONF_PROTOCOL_VERSION));
    expected_responses.add(&hello_response, false);
    WvStringList expected_get_response;
    expected_get_response.append("ONEVAL cfg/pickles/apples/foo 1");
    expected_responses.add(&expected_get_response, false);
    WvStringList expected_subt_response;
    expected_subt_response.append("VAL cfg {}");
    expected_responses.add(&expected_subt_response, false);

    WvUnixAddr addr(slavepipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonTestConn conn(sock, &commands, &expected_responses);
    
    WvIStreamList::globallist.append(&conn, false, "conn");

    printf("Spinning: streams left: %i\n", 
           WvIStreamList::globallist.count());
    while (!WvIStreamList::globallist.isempty() && 
            conn.isok())
    {
        printf("Spinning: streams left: %i\n", 
               WvIStreamList::globallist.count());
        WvIStreamList::globallist.runonce();
    }

    WvIStreamList::globallist.zap();
    WVRELEASE(master);
    WVRELEASE(slave);

    unlink(slavepipename.cstr());
    unlink(masterpipename.cstr());
    unlink(ininame.cstr());
}


WVTEST_MAIN("daemon proxying - with cache")
{
    signal(SIGPIPE, SIG_IGN);

    daemon_proxy_test(false);
}


WVTEST_MAIN("daemon proxying - cfg the only mount")
{
    signal(SIGPIPE, SIG_IGN);
    
    daemon_proxy_test(true);
}

