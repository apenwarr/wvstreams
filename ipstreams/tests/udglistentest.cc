/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * WvUnixDGListener test.  Creates a Unix Domain socket on /tmp/fuzzy or
 * whatever you give it on the command line, prints anything it receives to
 * stdout.
 */
#include "wvunixdgsocket.h"
#include "wvlog.h"
#include "wvistreamlist.h"

#define DEFAULT "/tmp/fuzzy"

int main(int argc, char **argv ) 
{
    WvLog log(("unixdatagramlisten"), WvLog::Debug1);

    WvString filename;
    if (argc == 2) 
        filename = argv[1];
    else 
        filename = DEFAULT;
        

    WvUnixDGListener sock(filename, 0777);

    sock.autoforward(*wvout);

    log("Listening on %s\n", filename);
   
    WvIStreamList l;
    l.add_after(l.tail, &sock, false, "socket");
    l.add_after(l.tail, wvcon, false, "wvcon");

    while (sock.isok() && wvout->isok()) 
    {
        if (l.select(-1))
            l.callback();
    }

    return 0;
}
