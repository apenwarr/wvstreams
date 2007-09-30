/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * simpleconn is a very simple program that forwards data between an
 * arbitrary set of given files (which are usually devices, named pipes,
 * etc).  If a file is named '-', it refers to stdin/stdout.
 */
#include "wvfile.h"
#include "wvistreamlist.h"
#include "wvtcp.h"
#include "wvlog.h"
#include <signal.h>


static void bouncer(WvStream *input, WvIStreamList *list)
{
    WvIStreamList::Iter i(*list);
    char buf[1024];
    size_t len;
    
    if (!input->select(0, true, false, false))
	return;
    
    len = input->read(buf, sizeof(buf));
    if (!len) return;
    
    for (i.rewind(); i.next(); )
    {
	IWvStream &out = i();
	
	if (input == &out)
	    continue;
	
	if (!out.isok())
	    continue;
        
	//delay_output wasn't really useful, and isn't in IWvStream
	//out.delay_output(true);
	out.write(buf, len);
    }
}


static void accept_callback(WvTCPListener *listen, WvIStreamList *list)
{
    WvTCPConn *conn = listen->accept();
    conn->setcallback(wv::bind(bouncer, conn, list));
    list->append(conn, true, "WvTCPConn");
}


int main(int argc, char **argv)
{
    WvIStreamList biglist, l;
    int count;
    char *cptr;
    WvLog log("simpleconn", WvLog::Info);
    
    signal( SIGPIPE, SIG_IGN );

    if (argc < 2)
    {
	fprintf(stderr, "Usage: %s file [file...]\n"
		"     Passes data between the given files/devices/etc.  "
		"If a filename is\n"
		"     '-', it refers to stdin/stdout.\n",
		argv[0]);
	return 1;
    }
    
    biglist.append(&l, false, "list");
    
    for (count = 1; count < argc; count++)
    {
	WvStream *f;
	
	if (!strcmp(argv[count], "-"))
	{
	    log("File %s is stdin/stdout\n", count);
	    f = wvcon;
	    l.append(f, false, "wvcon");
	    f->setcallback(wv::bind(bouncer, f, &l));
	}
	else if (!strncasecmp(argv[count], "tcp:", 4))
	{
	    // TCP connection of some kind
	    cptr = argv[count] + 4;
	    if (strchr(cptr, ':')) // an additional colon? client mode.
	    {
		log("File %s is a TCP client\n", count);
		f = new WvTCPConn(WvIPPortAddr(cptr));
		l.append(f, true, "TCP client");
		f->setcallback(wv::bind(bouncer, f, &l));
	    }
	    else // server mode
	    {
		log("File %s is a TCP server\n", count);
		WvTCPListener *listen = new WvTCPListener(WvIPPortAddr("",
							 atoi(cptr)));
		listen->setcallback(wv::bind(accept_callback, listen, &l));
		biglist.append(listen, true, "TCP server");
	    }
	}
	else
	{
	    log("File %s is a file (%s)\n", count, argv[count]);
	    f = new WvFile(argv[count], O_RDWR);
	    
	    if (!f->isok())
	    {
		WVRELEASE(f);
		f = new WvFile(argv[count], O_RDONLY);
		if (!f->isok())
		{
		    fprintf(stderr, "%s: %s\n",
			    argv[count], f->errstr().cstr());
		    return 1;
		}
	    }
	    
	    l.append(f, true, "file");
	    f->setcallback(wv::bind(bouncer, f, &l));
	}
    }
    
    
    // continue as long as there is more than one open client, or a server
    while (l.count() >= 2 || (biglist.count() - 1 >= 1 && l.count() >= 1))
    {
	if (biglist.select(1000))
	    biglist.callback();
    }
    
    return 0;
}
