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
    WvTCPConn sock(WvString(argc==2 ? argv[1] : "127.0.0.1:25"));
    char *line;
    char buf[500];
    size_t len;
    
    err("Starting...\n");
    
    // wvcon->autoforward(sock);
    // sock.autoforward(*wvcon);
    
    WvIStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
    
    while (wvcon->isok() && sock.isok())
    {
        l.runonce(-1);
        if ((len = sock.read(buf, sizeof(buf))) > 0)
        {
            err("From socket: %s bytes\n", len);
            wvcon->write(buf, len);
        }
        else
            err("Socket has no data.\n");
        if ((line = wvcon->getline()) != NULL)
        {
            err("Line from stdin: '%s'\n", line);
            sock.print("%s\n", line);
        }
        else
            err("Console has no data.\n");
    }
    
    err("End of loop.\n");
    
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", wvcon->errstr());
    if (!sock.isok() && sock.geterr())
	err("socket: %s\n", sock.errstr());
    
    return 0;
}
