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
