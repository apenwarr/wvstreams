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
    "ftp://ftp.test.test",
    "ftp://monkey@ftp.test.test",
    "ftp://monkey:banana@ftp.test.test/file",
    NULL
};

int main()
{
    char **s;
    for (s = urls; *s != NULL; s++)
    {
        WvUrl url(*s);
        WvUrl url2(url);
        wvcon->print("%s -> %s\n", *s, url2);
        wvcon->print("proto: %s, host: %s, port:%s, file: %s, user: %s, password: %s\n",
                     url2.getproto(), url2.gethost(), url2.getport(),
                     url2.getfile(), url2.getuser(), url2.getpassword());
        wvcon->print("\n");
    }
}
