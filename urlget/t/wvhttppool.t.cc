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
    {
	if (l.select(-1))
	{
	    l.callback();
	}
    }
    WVPASS(!(buf->isok()));
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


void listener_callback(WvStream &s, void*)
{
    WvTCPListener &l = (WvTCPListener &)s;
    http_conns++;
    wvcon->write("Incoming connection (%s)\n", http_conns);
    WvTCPConn *newconn = l.accept();
    newconn->setcallback(tcp_callback, NULL);
    WvIStreamList::globallist.append(newconn, true);
}


void do_test()
{
    WvHttpPool pool;
    WvIStreamList::globallist.append(&pool, false);

    http_conns = 0;
    got_first_pipeline_test = false;
    WvStream *buf;
    WVPASS(buf = pool.addurl("http://localhost:4200"));
    WVPASS(buf->isok());
    buf->autoforward(*wvcon);
    buf->setclosecallback(close_callback, NULL);
    WvIStreamList::globallist.append(buf, true);
    while (buf->isok() && (wvcon->isok() || !pool.idle()))
    {
	if (WvIStreamList::globallist.select(-1))
	    WvIStreamList::globallist.callback();
    }
    WVPASS(!(buf->isok()));
}


WVTEST_MAIN("WvHttpPool pipelining")
{
    WvTCPListener listener(4200);
    listener.setcallback(listener_callback, NULL);
    WvIStreamList::globallist.append(&listener, false);

    do_test();
    WVPASS(http_conns == 1);

    pipelining_enabled = false;
    do_test();
    WVPASS(http_conns == 2);
}
