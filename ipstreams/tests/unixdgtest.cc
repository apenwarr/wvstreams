/*
 * Worldvisions Weaver Software: Copyright (C) 1997-2004 Net Integration
 * Technologies, Inc.
 *
 * WvUnixDGConn test.  Outputs to a unix datagram socket on  /tmp/fuzzy or
 * whatever you give it on the command line.  Note that the destination socket
 * must exist beforehand.
 */
#include "wvunixdgsocket.h" 
#include "wvlog.h"
#include "wvistreamlist.h"

#define DEFAULT "/tmp/fuzzy"

int main(int argc,  char **argv)
{
    WvLog err("unixdgtest", WvLog::Error);
    
    WvString filename;
    if (argc == 2)
        filename = argv[1];
    else
        filename = DEFAULT;
    
    WvUnixDGConn sock(filename);
   
    wvcon->autoforward(sock);
    sock.autoforward(*wvcon);

    WvIStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false); 

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
