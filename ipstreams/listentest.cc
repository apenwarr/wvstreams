/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvtcp.h"
#include "wvstreamlist.h"
#include "wvlog.h"

static int stream_bounce_to_list(WvStream &s, void *userdata)
{
    WvStreamList &l = *(WvStreamList *)userdata;
    WvStreamList::Iter i(l);
    WvStream *out;
    char *line;

    while ((line = s.getline(0)) != NULL)
    {
	if (!strncmp(line, "quit", 4))
	{
	    s.close();
	    continue;
	}
	
	for (i.rewind(); i.next(); )
	{
	    out = i.data();
	    if (out != &s && out->select(0, false, true))
		out->print("%s> %s\n", 
			   s.src() ? (WvString)*s.src() : WvString("stdin"),
			   line);
	}
    }
    
    return 0;
}


int main(int argc, char **argv)
{
    {
	WvLog log("testlisten"), err = log.split(WvLog::Error);
	WvStream wvin(0), wvout(1);
	WvStreamList l;

	WvTCPListener sock(WvIPPortAddr(argc==2 ? argv[1] : "0.0.0.0:4242"));
	
	wvin.setcallback(stream_bounce_to_list, &l);
	sock.auto_accept(&l, stream_bounce_to_list, &l);
	
	log("Listening on port %s\n", *sock.src());
	
	l.append(&sock, false);
	l.append(&wvin, false);
	
	while (sock.isok() && wvin.isok())
	{
	    if (l.select(-1))
		l.callback();
	}
	
	if (!sock.isok() && sock.geterr())
	    err("%s\n", strerror(sock.geterr()));
    }
    // all variables should now be freed
	
#if DEBUG
    fprintf(stderr, "File descriptors still open: ");

    bool found = false;
    for (int count = 0; count < 255; count++)
    {
	int dupfd = dup(count);
	if (dupfd >= 0)
	{
	    fprintf(stderr, "#%d ", count);
	    found = true;
	    close(dupfd);
	}
    }

    if (!found)
	fprintf(stderr, "none.\n");
    else
	fprintf(stderr, "\n");
#endif
    
    return 0;
}
