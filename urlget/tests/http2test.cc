/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvHttpPool test.  Downloads the urls given on stdin.
 */
#include "wvhttppool.h"
#include "wvfile.h"
#include "strutils.h"
#include <signal.h>


bool want_to_die = false;

void sighandler_die(int signum)
{
    fprintf(stderr, "Caught signal %d; cleaning up and terminating.\n",
	    signum);
    want_to_die = true;
    signal(signum, SIG_DFL);
}


int main(int argc, char **argv)
{
    WvLog log("http2test", WvLog::Info);
    WvStreamList l;
    WvHttpPool p;
    WvString headers("");
    char *line;
    
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighandler_die);
#endif
    
    l.append(wvcon, false);
    l.append(&p, false);
    
    while (!want_to_die && p.isok() && (wvcon->isok() || !p.idle()))
    {
	if (l.select(1000))
	{
	    l.callback();
	    
	    line = wvcon->getline(0);
	    if (line)
	    {
		line = trim_string(line);
		if (!line[0])
		    continue;
		else if (strstr(line, ": "))
		{
		    // an extra http header
		    headers = WvString("%s%s\n", headers, line);
		    continue;
		}
		
		WvStream *s = p.addurl(line, "GET", headers);
		if (s)
		{
		    static int num = 0;
		    WvFile *f = new WvFile(WvString("/tmp/url_%s", ++num), 
					   O_CREAT|O_WRONLY|O_TRUNC);
		    assert(!f->readable);
		    s->autoforward(*f);
		    l.append(s, true);
		    l.append(f, true);
		}
	    }
	}
    }
    
    if (!p.isok() && p.geterr())
	log("HttpPool: %s\n", p.errstr());
    
    return 0;
}
