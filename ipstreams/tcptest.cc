/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvtcp.h"
#include "wvstreamlist.h"
#include "wvlog.h"


int main(int argc, char **argv)
{
    WvLog err("tcptest", WvLog::Error);
    WvIPPortAddr remaddr(argc==2 ? argv[1] : "0.0.0.0:25");
    WvTCPConn sock(remaddr);
    
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);
    
    WvStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
    
    while (wvcon->isok() && sock.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", strerror(wvcon->geterr()));
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", strerror(sock.geterr()));
    
    return 0;
}
