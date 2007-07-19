/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvUnixConn test.  Creates a Unix Domain socket on /tmp/fuzzy or whatever
 * you give it on the command line.
 */
#include "wvunixsocket.h"
#include "wvistreamlist.h"
#include "wvlog.h"


int main(int argc, char **argv)
{
    WvLog err("unixtest", WvLog::Error);
    WvUnixConn sock(argc==2 ? argv[1] : "/tmp/fuzzy");
    
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);
    
    WvIStreamList l;
    l.append(wvcon, false, "wvcon");
    l.append(&sock, false, "socket");
    
    while (wvcon->isok() && sock.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", wvcon->errstr());
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", sock.errstr());
    
    return 0;
}
