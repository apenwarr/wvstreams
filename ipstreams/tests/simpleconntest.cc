/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * simpleconn is a very simple program that forwards data between an
 * arbitrary set of given files (which are usually devices, named pipes,
 * etc).  If a file is named '-', it refers to stdin/stdout.
 */
#include "wvfile.h"
#include "wvstreamlist.h"
#include "wvtcp.h"
#include "wvlog.h"
#include <signal.h>


static void bouncer(WvStream &s, void *userdata)
{
    WvStreamList &l = *(WvStreamList *)userdata;
    WvStreamList::Iter i(l);
    char buf[1024];
    size_t len;
    
    if (!s.select(0, true, false, false))
	return;
    
    len = s.read(buf, sizeof(buf));
    if (!len) return;
    
    for (i.rewind(); i.next(); )
    {
	WvStream &out = *i;
	
	if (&s == &out)
	    continue;
	
	if (!out.isok())
	    continue;
	
	out.delay_output(true);
	out.write(buf, len);
    }
}


int main(int argc, char **argv)
{
    WvStreamList biglist, l;
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
    
    biglist.append(&l, false);
    
    for (count = 1; count < argc; count++)
    {
	WvStream *f;
	
	if (!strcmp(argv[count], "-"))
	{
	    log("File %s is stdin/stdout\n", count);
	    f = wvcon;
	    l.append(f, false);
	    f->setcallback(bouncer, &l);
	}
	else if (!strncasecmp(argv[count], "tcp:", 4))
	{
	    // TCP connection of some kind
	    cptr = argv[count] + 4;
	    if (strchr(cptr, ':')) // an additional colon? client mode.
	    {
		log("File %s is a TCP client\n", count);
		f = new WvTCPConn(WvIPPortAddr(cptr));
		l.append(f, true);
		f->setcallback(bouncer, &l);
	    }
	    else // server mode
	    {
		log("File %s is a TCP server\n", count);
		WvTCPListener *listen = new WvTCPListener(WvIPPortAddr("",
							 atoi(cptr)));
		listen->auto_accept(&l, bouncer, &l);
		biglist.append(listen, true);
	    }
	}
	else
	{
	    log("File %s is a file (%s)\n", count, argv[count]);
	    f = new WvFile(argv[count], O_RDWR);
	    
	    if (!f->isok())
	    {
		f->release();
		f = new WvFile(argv[count], O_RDONLY);
		if (!f->isok())
		{
		    fprintf(stderr, "%s: %s\n",
			    argv[count], f->errstr().cstr());
		    return 1;
		}
	    }
	    
	    l.append(f, true);
	    f->setcallback(bouncer, &l);
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
