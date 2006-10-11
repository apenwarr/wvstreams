/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvTCPConn test.  Telnets to your local SMTP port, or any other port given
 * on the command line.
 */
#include "wvtcp.h"
#include "wvistreamlist.h"
#include "wvlog.h"

int main(int argc, char **argv)
{
    WvLog err("tcptest", WvLog::Error);
    err(WvLog::Debug1, "tcptest starting.\n");
    
    WvTCPConn sock(WvString(argc==2 ? argv[1] : "127.0.0.1:25"));
    
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);
    
    WvIStreamList l;
    l.add_after(l.tail, wvcon, false, "wvcon");
    l.add_after(l.tail, &sock, false, "socket");
    
    while (wvcon->isok() && sock.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", wvcon->errstr());
    if (!sock.isok() && sock.geterr())
	err("socket: %s\n", sock.errstr());
    err(WvLog::Debug1, "tcptest exiting.\n");
    
    return 0;
}
