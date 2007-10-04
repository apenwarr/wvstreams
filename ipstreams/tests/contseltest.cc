/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvStream test using the new WvTask support.  This is in ipstreams
 * because there are more fun and stressful streams to test here.
 */
#include "wvtcp.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "strutils.h"
#include "wvcont.h"

static void *stream_call(WvStream& s)
{
    char *line;
    static int sc_count = 0;
    int mynum = ++sc_count, count = 0;
    WvLog log(WvString("log%s", mynum), WvLog::Info);
    
    while (s.isok())
    {
	if ((line = s.getline()) != NULL)
	{
	    line = trim_string(line);
	    s.print("%s/%s: You said: '%s'\n", mynum, count, line);
	    log("#%s: You said: '%s'\n", count, line);
	}
	else
	{
	    log("#%s: Tick.\n", count);
	    s.print("!");
	}
	
	count++;
	s.continue_select(100*mynum);
    }

    return 0;
}


static void setupcont_call(WvIStreamList *list, IWvStream *_conn)
{
    WvStreamClone *conn = new WvStreamClone(_conn);
    conn->setcallback(WvCont(wv::bind(&stream_call, wv::ref(*conn))));
    list->append(conn, true, "WvTCPConn");
}


int main()
{
    WvLog log("conttest"), err = log.split(WvLog::Error);
    WvIStreamList l;
    
    WvTCPListener listen(WvIPPortAddr("0.0.0.0:1129"));
    listen.onaccept(wv::bind(setupcont_call, &l, wv::_1));

    wvcon->setcallback(WvCont(wv::bind(&stream_call, wv::ref(*wvcon))));

    log("Listening on port %s\n", *listen.src());
    
    l.append(&listen, false, "listener");
    l.append(wvcon, false, "wvcon");
    
    while (listen.isok() && wvcon->isok())
    {
	log("main loop\n");
	if (l.select(10000))
	    l.callback();
    }
    
    if (!listen.isok() && listen.geterr())
	err("%s\n", listen.errstr());
	
    return 0;
}
