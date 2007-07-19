/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * WvIPRawStream test.  Waits for data.
 */

#include <netinet/in.h>

#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvipraw.h"

int main(int argc, char **argv)
{
    WvLog err("iptest", WvLog::Error);
    WvIPAddr nothing;
    WvIPAddr remaddr(argc > 1 ? argv[1] : "127.0.0.1");
    WvIPRawStream sock(nothing, remaddr, IPPROTO_ESP);
    
    sock.enable_broadcasts();
    
    err(WvLog::Info, "Local address is %s.\n", *sock.local());
    
    wvcon->autoforward(sock);
    sock.autoforward(err);
    
    WvIStreamList l;
    l.append(wvcon, false, "wvcon");
    l.append(&sock, false, "socket");
    
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
