/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvUDPStream test.  Waits for data on port 19.
 */

#include "wvurl.h"
#include "wvstream.h"

char *urls[] = {
    "http://www.test.test",
    "http://www.test.test:100",
    "http://www.test.test:80",
    "sip://www.test.test",
    "sip://www.test.test:80",
    "sip:www.test.test",
    "test of a bad proto",
    "test://www.test.test:100",
    "http",
    "http: but no slashes",
    NULL
};

int main()
{
    char **s;
    for (s = urls; *s != NULL; s++)
    {
        WvUrl url(*s);
        wvcon->print("%s -> %s\n", *s, url);
        wvcon->print("proto: %s, host: %s, port:%s, file: %s, user: %s\n",
                     url.getproto(), url.gethost(), url.getport(),
                     url.getfile(), url.getuser());
        wvcon->print("\n");
    }
}
