/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * WvStream test using the new WvTask support.  This is in ipstreams
 * because there are more fun and stressful streams to test here.
 */
#include "wvtcp.h"
#include "wvstreamlist.h"
#include "wvlog.h"
#include "strutils.h"

static void stream_call(WvStream &s, void *userdata)
{
    if (!s.uses_continue_select)
    {
	s.uses_continue_select = true;
	return;
    }
    
    char *line;
    static int sc_count = 0;
    int mynum = ++sc_count, count = 0;
    WvLog log(WvString("log%s", mynum), WvLog::Info);
    
    while (s.isok())
    {
	if ((line = s.getline(0)) != NULL)
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
}


int main()
{
    WvLog log("conttest"), err = log.split(WvLog::Error);
    WvStreamList l;
    
    WvTCPListener listen(WvIPPortAddr("0.0.0.0:1129"));
    listen.auto_accept(&l, stream_call, NULL);
    
    wvcon->setcallback(stream_call, NULL);
    
    log("Listening on port %s\n", *listen.src());
    
    l.append(&listen, false);
    l.append(wvcon, false);
    
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
