#include "wvtest.h"
#include "wvhttppool.h"
#include <stdio.h>

static void close_callback(WvStream& s)
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
    buf->setclosecallback(close_callback);
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
    buf->setclosecallback(close_callback);
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

bool pipelining_enabled = true;
bool expecting_request = false;
bool break_connection = false;
unsigned int http_conns = 0;

void tcp_callback(WvStream &s, void*)
{
    bool last_was_pipeline_check = false;
    WvString buf("");
    char *line;
    do
    {
        line = s.getline(0);
        if (line && (strncmp(line, "GET", 3) == 0 ||
                     strncmp(line, "HEAD", 4) == 0))
        {
            if (expecting_request)
            {
                if (strstr(line, "wvhttp-pipeline-check-should-not-exist"))
                {
                    printf("Sending 404\n");
                    buf.append("HTTP/1.1 404 Not Found\n\n");
                    last_was_pipeline_check = true;
                }
                else
                {
                    printf("Sending 200\n");
                    buf.append("HTTP/1.1 200 OK\n"
                               "Content-Length: 5\n"
                               "Content-Type: text/html\n\n"
                               "Foo!\n");
                    last_was_pipeline_check = false;
                    if (break_connection)
                    {
                        break_connection = false;
                        s.close();
                    }
                }
                if (!pipelining_enabled)
                    expecting_request = false;
            }
            else
            {
                printf("Sending 400\n");
                buf.append("HTTP/1.1 400 Invalid Request\n"
                           "Content-Length: 5\n"
                           "Content-Type: text/html\n\n"
                           "Bar!\n");
                // we should only be returning a 400 during the pipeling test;
                // otherwise, it means that WvHttpPool didn't detect broken
                // pipelining correctly.
                WVPASS(last_was_pipeline_check);
            }
        }
    } while (line);
    s.print(buf);
    expecting_request = true;
}


void listener_callback(WvStream &s, void *userdata)
{
    WvIStreamList &list = *(WvIStreamList *)userdata;
    WvTCPListener &l = (WvTCPListener &)s;
    http_conns++;
    WvTCPConn *newconn = l.accept();
    printf("Incoming connection (%u)\n", http_conns);
    newconn->setcallback(tcp_callback, NULL);
    list.append(newconn, false, "incoming http conn");
    expecting_request = true;
}


static void do_test(WvIStreamList &l, unsigned int num_requests)
{
    printf("pipelining [%d] requusts [%u]\n", pipelining_enabled,
           num_requests);
    WvHttpPool pool;
    WvIStreamList bufs;
    l.append(&pool, false);
    l.append(&bufs, false);

    http_conns = 0;
    WvStream *buf;
    for (unsigned int i = 0; i < num_requests; i++)
    {
        WVPASS(buf = pool.addurl(WvString("http://localhost:4200/%s.html", i)));
        WVPASS(buf->isok());
        buf->autoforward(*wvcon);
        buf->setclosecallback(close_callback);
        bufs.append(buf, true, "poolbuf");
    }

    WvIStreamList::Iter j(bufs);
    while (wvcon->isok() || !pool.idle())
    {
        bool buf_ok = false;
        for (j.rewind(); j.next(); )
            if (j().isok())
                buf_ok = true;

        if (!buf_ok)
            break;

	l.runonce();
    }

    l.runonce(10);
    l.runonce(10);
    WVPASSEQ(bufs.count(), 0);
    l.unlink(&bufs);
    l.unlink(&pool);
}


#if 0
WVTEST_MAIN("WvHttpPool pipelining")
{
    WvIStreamList l;
    
    WvTCPListener listener(4200);
    listener.setcallback(listener_callback, &l);
    l.append(&listener, false, "http listener");

    // Pipelining-enabled tests share one connection for the pipeline test
    // and actual requests.
    do_test(l, 1);
    WVPASSEQ(http_conns, 1);
    do_test(l, 5);
    WVPASSEQ(http_conns, 1);

    break_connection = true;
    do_test(l, 1);
    WVPASSEQ(http_conns, 2);
    break_connection = true;
    do_test(l, 5);
    WVPASSEQ(http_conns, 2);

    // All pipelining-disabled tests should have one connection for the
    // pipeline test and one for the real connection.
    pipelining_enabled = false;
    do_test(l, 1);
    WVPASSEQ(http_conns, 2);
    do_test(l, 5);
    WVPASSEQ(http_conns, 2);

    break_connection = true;
    do_test(l, 1);
    WVPASSEQ(http_conns, 3);
    break_connection = true;
    do_test(l, 5);
    WVPASSEQ(http_conns, 3);

    WVPASS(listener.isok());
}
#endif
