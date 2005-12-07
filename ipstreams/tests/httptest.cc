/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvHTTPStream test.  Downloads a file via http.
 */
#include "wvhttp.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvfile.h"


int main(int argc, char **argv)
{
    WvLog log("httptest", WvLog::Info);
    WvURL url("http://www.net-itech.com/");
    WvHTTPStream http(url);
    WvFile out("httptest.out", O_WRONLY | O_TRUNC | O_CREAT);
    WvHTTPStream::State last_state = WvHTTPStream::Done;
    static char buf[10240];
    size_t len;
    
    WvIStreamList l;
    l.add_after(l.tail, &http, false, "WvHTTPStream");
    
    while (http.isok() && out.isok())
    {
	if (last_state != http.state)
	{
	    log("\nNow in state %s\n", http.state);
	    last_state = http.state;
	}
	    
	if (l.select(100))
	    l.callback();
	    
	if (http.select(0))
	{
	    len = http.read(buf, sizeof(buf));
	    out.write(buf, len);
	    log("[%6s]", len);
	}
    }
    
    if (!http.isok() && http.geterr())
	log("http: %s\n", http.errstr());
    
    return 0;
}
