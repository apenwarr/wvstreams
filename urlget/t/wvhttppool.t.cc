#include "wvtest.h"
#include "wvhttppool.h"
#include <stdio.h>

static void close_callback(WvStream& s, void* userdata)
{
    if (!s.isok())
        printf("%d", s.geterr());
}

WVTEST_MAIN("WvHttpPool GET")
{
    WvHttpPool pool;

    WvIStreamList l;
    l.append(&pool, false);

    WvStream *buf;
    WVPASS(buf = pool.addurl("http://www.google.ca"));
    WVPASS(buf->isok());
    buf->autoforward(*wvcon);
    buf->setclosecallback(close_callback, NULL);
    l.append(buf, true);
    while (buf->isok() && (wvcon->isok() || !pool.idle()))
	l.runonce();
    WVPASS(!(buf->isok()));
    
    WVPASSEQ(l.count(), 2);
}

WVTEST_MAIN("WvHttpPool HEAD")
{
    WvHttpPool pool;

    WvIStreamList l;
    l.append(&pool, false);

    WvStream *buf;
    WVPASS(buf = pool.addurl("http://www.google.ca", "HEAD"));
    WVPASS(buf->isok());
    buf->autoforward(*wvcon);
    buf->setclosecallback(close_callback, NULL);
    l.append(buf, true);
    while (buf->isok() && (wvcon->isok() || !pool.idle()))
    {
	if (l.select(-1))
	{
	    l.callback();
	}
    }
    WVPASS(!(buf->isok()));
}


bool got_first_pipeline_test = false;
bool pipelining_enabled = true;
unsigned int http_conns = 0;

void tcp_callback(WvStream &s, void*)
{
    WvLog log("tcp_callback", WvLog::Info);
    char *line = s.getline(0);
    if (line && (strncmp(line, "GET", 3) == 0 || strncmp(line, "HEAD", 4) == 0))
    {
        if (strstr(line, "wvhttp-pipeline-check-should-not-exist"))
        {
            log("Sending 404\n");
            s.print("HTTP/1.1 404 Not Found\n\n");
            got_first_pipeline_test = true;
        }
        else if (got_first_pipeline_test && !pipelining_enabled)
            s.print("HTTP/1.1 400 Invalid Request\n"
                    "Content-Length: 5\n"
                    "Content-Type: text/html\n\n"
                    "Bar!\n");
        else
        {
            log("Sending 200\n");
            s.print("HTTP/1.1 200 OK\n"
                    "Content-Length: 5\n"
                    "Content-Type: text/html\n\n"
                    "Foo!\n");
        }
    }
}


void listener_callback(WvStream &s, void *userdata)
{
    WvIStreamList &list = *(WvIStreamList *)userdata;
    WvTCPListener &l = (WvTCPListener &)s;
    http_conns++;
    wvcon->write("Incoming connection (%s)\n", http_conns);
    WvTCPConn *newconn = l.accept();
    newconn->setcallback(tcp_callback, NULL);
    list.append(newconn, true, "incoming http conn");
}


static void do_test(WvIStreamList &l)
{
    WvHttpPool pool;
    l.append(&pool, false, "pool");

    http_conns = 0;
    got_first_pipeline_test = false;
    WvStream *buf;
    WVPASS(buf = pool.addurl("http://localhost:4200"));
    WVPASS(buf->isok());
    buf->autoforward(*wvcon);
    buf->setclosecallback(close_callback, NULL);
    l.append(buf, true, "poolbuf");
    while (buf->isok() && (wvcon->isok() || !pool.idle()))
	l.runonce();
    WVPASS(!(buf->isok()));
    l.unlink(&pool);
}


WVTEST_MAIN("WvHttpPool pipelining")
{
    WvIStreamList l;
    
    WvTCPListener listener(4200);
    listener.setcallback(listener_callback, &l);
    l.append(&listener, false, "http listener");

    do_test(l);
    WVPASSEQ(http_conns, 1);

    pipelining_enabled = false;
    do_test(l);
    WVPASSEQ(http_conns, 2);
    
    l.unlink(&listener);
    l.runonce(10);
    l.runonce(10);
    
    // list should now be empty
    WVPASSEQ(l.count(), 0);
}
