/** \file
 * A WvHttpStream example.
 */
/** \example wvhttpex.cc
 * This program downloads a file via http.
 * The expected result is:
 * http<Info>: Now in state 0
 * http<Info>: Now in state 1
 * http<Info>: [     0]
 * http<Info>: Now in state 2
 * http<Info>: [     0]
 * http<Info>: Now in state 3
 * http<Info>: [     0][     0][     0][     0][     0][     0][     0][     0][     0]
 * http<Info>: Now in state 4
 * http<Info>: [   751][   922][     0]
 *
 */
#include "wvhttp.h"
#include "wvstreamlist.h"
#include "wvlog.h"
#include "wvfile.h"


int main(int argc, char **argv)
{
    WvLog log("http", WvLog::Info);
    WvURL url("http://www.net-itech.com/");
    WvHTTPStream http(url);
    WvFile out("http.out", O_WRONLY | O_TRUNC | O_CREAT);
    WvHTTPStream::State last_state = WvHTTPStream::Done;
    static char buf[10240];
    size_t len;

    WvStreamList l;
    l.add_after(l.tail, &http, false);

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
