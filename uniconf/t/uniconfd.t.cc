#include "wvtest.h"
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
#include <signal.h>

/**** Generic daemon testing helpers ****/

DeclareWvList(WvStringList);

class UniConfDaemonClientConn : public WvStreamClone
{
public:
    UniConfDaemonClientConn(IWvStream *s, WvStringList *_commands, 
                            WvStringListList *_expected_responses) :        
        WvStreamClone(s),
        commands(_commands),
        expected_responses(_expected_responses),
        log("UniConfDaemonClientConn", WvLog::Debug)      
        {
            setclone(s);
            uses_continue_select = true;
        }
    virtual ~UniConfDaemonClientConn() 
        {
            log("Destructing\n");
            terminate_continue_select();            
        }

    virtual void close()
        {
            log("Closing connection\n");
            cloned->close();
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
                close();
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
    printf("Awaiting '%s' '%s' '%s'\n", w1, w2, w3);
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
    hello_response.append("HELLO {UniConf Server ready.}");
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

    WvString pipename("/tmp/tmpfile1");
    WVPASS(daemon.setupunixsocket(pipename));
    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonClientConn conn(sock, &commands, &expected_responses);

    WvIStreamList::globallist.append(&conn, false);
    WvIStreamList::globallist.append(&daemon, false);
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
    hello_response.append("HELLO {UniConf Server ready.}");
    expected_responses.add(&hello_response, false);
    WvStringList expected_quit_response;
    expected_quit_response.append("OK ");
    expected_responses.add(&expected_quit_response, false);

    WvString pipename("/tmp/tmpfile1");
    WVPASS(daemon.setupunixsocket(pipename));
    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonClientConn conn(sock, &commands, &expected_responses);

    WvIStreamList::globallist.append(&conn, false);
    WvIStreamList::globallist.append(&daemon, false);
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
// e.g.: client -> uniconfd -> uniconfd 519-496-0675

const char * const uniconfd1_args[] = {
    "uniconf/daemon/uniconfd",
    "-f",
    "-p",
    "0",
    "-s"
    "0",
    "-u",
    "/tmp/tmpfile1",
    "/cfg=ini:/tmp/dumb.ini",
    NULL
};

const char * const uniconfd1_2_args[] = {
    "uniconf/daemon/uniconfd",
    "-f",
    "-p",
    "0",
    "-s"
    "0",
    "-u",
    "/tmp/tmpfile1",
    "/=temp:",
    "/cfg=ini:/tmp/dumb.ini",
    NULL
};

WvPipe * setup_master_daemon(bool implicit)
{
    WvFile stuff("/tmp/dumb.ini", O_WRONLY|O_TRUNC);
    stuff.print("pickles/apples/foo=1\n");
    stuff.print("pickles/mangos/bar=1\n");
    stuff.close();

    return new WvPipe(uniconfd1_args[0], 
                      implicit ? uniconfd1_args : uniconfd1_2_args,
                      false, false, false);
}

const char * const uniconfd2_args[] = {
    "uniconf/daemon/uniconfd",
    "-f",
    "-p",
    "0",
    "-s"
    "0",
    "-u",
    "/tmp/tmpfile2",
    "/=retry:unix:/tmp/tmpfile1",
    NULL
};

const char * const uniconfd2_2_args[] = {
    "uniconf/daemon/uniconfd",
    "-f",
    "-p",
    "0",
    "-s"
    "0",
    "-u",
    "/tmp/tmpfile2",
    "/=retry:cache:unix:/tmp/tmpfile1",
    NULL
};

void daemon_proxy_test(bool implicit)
{
    WvString pipename = "/tmp/tmpfile2";

    WvPipe *master = setup_master_daemon(implicit);
    master->setcallback(WvPipe::ignore_read, NULL);
    master->nowrite();
    sleep(1);
    fprintf(stderr, "Got here (1)\n");

    WvPipe *slave = new WvPipe(uniconfd2_args[0], 
                               implicit ? uniconfd2_args : uniconfd2_2_args, 
                               false, false, false); 
    slave->setcallback(WvPipe::ignore_read, NULL);
    slave->nowrite();
    sleep(1);

    fprintf(stderr, "Got here (2)\n");
    WvStringList commands;
    commands.append("get /cfg/pickles/apples/foo");
    commands.append("subt /");
    WvStringListList expected_responses;
    WvStringList hello_response;
    hello_response.append("HELLO {UniConf Server ready.}");
    expected_responses.add(&hello_response, false);
    WvStringList expected_get_response;
    expected_get_response.append("ONEVAL cfg/pickles/apples/foo 1");
    expected_responses.add(&expected_get_response, false);
    WvStringList expected_subt_response;
    expected_subt_response.append("VAL cfg {}");
    expected_responses.add(&expected_subt_response, false);

    fprintf(stderr, "Got here (3)\n");

    WvUnixAddr addr(pipename);
    WvUnixConn *sock = new WvUnixConn(addr);
    UniConfDaemonClientConn conn(sock, &commands, &expected_responses);
    
    WvIStreamList::globallist.append(&conn, false);

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
