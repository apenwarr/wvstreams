/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvTCPListener test.  Listens on a port, and bounces stdin and stdout 
 * between all connections established to it.
 */
#include "wvtcp.h"
#include "wvistreamlist.h"
#include "wvlog.h"


static void stream_bounce_to_list(WvStream &s, WvIStreamList *list)
{
    WvIStreamList::Iter out(*list);
    char *line;

    while ((line = s.getline()) != NULL)
    {
	if (!strncmp(line, "quit", 4))
	{
	    s.close();
	    continue;
	}
	
	for (out.rewind(); out.next(); )
	{
	    if (&out() != &s && out->iswritable())
	    {
		static_cast<WvStream*>(&out())->print("%s> %s\n", 
			    s.src() ? (WvString)*s.src() : WvString("stdin"),
			    line);
		if (s.src())
		    wvcon->print("Local address of source was %s\n",
				 ((WvTCPConn *)&s)->localaddr());
	    }
	}
    }
}


static void accept_callback(WvIStreamList &list, IWvStream *_conn)
{
    WvStreamClone *conn = new WvStreamClone(_conn);
    conn->setcallback(wv::bind(stream_bounce_to_list, wv::ref(*conn), &list));
    list.append(conn, true, "WvTCPConn");
}


int main(int argc, char **argv)
{
    {
	WvLog log("testlisten"), err = log.split(WvLog::Error);
	WvIStreamList l;

	WvTCPListener sock(WvIPPortAddr(argc==2 ? argv[1] : "0.0.0.0:0"));
	
	wvcon->setcallback(wv::bind(stream_bounce_to_list,
				    wv::ref(*wvcon), &l));
	sock.onaccept(wv::bind(accept_callback, wv::ref(l), _1));
	
	log("Listening on port %s\n", *sock.src());
	
	l.append(&sock, false, "socket");
	l.append(wvcon, false, "wvcon");
	
	while (sock.isok() && wvcon->isok())
	{
	    if (l.select(-1))
		l.callback();
	}
	
	if (!sock.isok() && sock.geterr())
	    err("%s\n", strerror(sock.geterr()));
	if (!wvcon->isok() && wvcon->geterr())
	    err("%s\n", strerror(wvcon->geterr()));
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
