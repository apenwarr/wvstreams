/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvudp.h"
#include "wvstreamlist.h"
#include "wvlog.h"

int main(int argc, char **argv)
{
    WvLog err("udptest", WvLog::Error);
    WvIPPortAddr localaddr(argc > 1 ? argv[1] : "0.0.0.0:4123"),
		   remaddr(argc > 2 ? argv[2] : "0.0.0.0");
    WvUDPStream sock(localaddr, WvIPPortAddr());
    
    err(WvLog::Info, "Bound to local address %s.\n", localaddr);
    
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);
    
    WvStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
    
    while (wvcon->isok() && sock.isok())
    {
	sock.setdest(remaddr);
	if (l.select(1000))
	{
	    l.callback();
	    err(WvLog::Info, "    (remote: %s)\n", *sock.src());
	}
	else
	    sock.print("%s\n", localaddr);
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", strerror(wvcon->geterr()));
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", strerror(sock.geterr()));
    
    return 0;
}
