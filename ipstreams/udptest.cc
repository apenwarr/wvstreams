/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
 */
#include "wvstreamlist.h"
#include "wvlog.h"
#include "wvudp.h"

int main(int argc, char **argv)
{
    WvLog err("udptest", WvLog::Error);
    WvIPPortAddr nothing;
    WvIPPortAddr remaddr(argc > 1 ? argv[1] : "127.0.0.1:19");
    WvUDPStream sock(nothing, nothing);
    
    err(WvLog::Info, "Local address is %s.\n", *sock.local());
    
    wvcon->autoforward(sock);
    sock.autoforward(err);
    
    WvStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
    
    while (wvcon->isok() && sock.isok())
    {
	sock.setdest(remaddr);
	if (l.select(1000))
	{
	    if (wvcon->select(0))
		wvcon->callback();
	    else if (sock.select(0))
	    {
		sock.callback();
		err(WvLog::Info, "    (remote: %s)\n", *sock.src());
	    }
	}
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", strerror(wvcon->geterr()));
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", strerror(sock.geterr()));
    
    return 0;
}
