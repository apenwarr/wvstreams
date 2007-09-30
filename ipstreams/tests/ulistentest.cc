/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvunixsocket.h"
#include "wvistreamlist.h"
#include "wvlog.h"

static void stream_bounce_to_list(WvStream *in, WvIStreamList *list,
				  WvStringParm localaddr)
{
    WvIStreamList::Iter out(*list);
    char *line;

    while ((line = in->getline()) != NULL)
    {
	if (!strncmp(line, "quit", 4))
	{
	    in->close();
	    continue;
	}
	
	for (out.rewind(); out.next(); )
	{
	    if (&out() != in && out->iswritable())
	    {
		static_cast<WvStream*>(&out())->print(
		    "%s> %s\n",
		    in->src() ? (WvString)*in->src() : WvString("stdin"),
		    line);
		if (localaddr)
		    wvcon->print("Local address of source was %s\n",
				 localaddr);
	    }
	}
    }
}


static void accept_callback(WvUnixListener *listen, WvIStreamList *list)
{
    WvUnixConn *conn = listen->accept();
    conn->setcallback(wv::bind(stream_bounce_to_list, conn, list,
			       conn->localaddr()));
    list->append(conn, true, "WvUnixConn");
}


int main(int argc, char **argv)
{
    {
	WvLog log("testlisten"), err = log.split(WvLog::Error);
	WvIStreamList l;

	WvUnixListener sock(argc==2 ? argv[1] : "/tmp/fuzzy",
			    0777);
	
	wvin->setcallback(wv::bind(stream_bounce_to_list, wvin, &l, NULL));
	sock.setcallback(wv::bind(accept_callback, &sock, &l));
	
	log("Listening on port %s\n", *sock.src());
	
	l.append(&sock, false, "socket");
	l.append(wvin, false, "wvin");
	
	while (sock.isok() && wvin->isok())
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
