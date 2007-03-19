/*
 * WvHTTPStream test.  Downloads a file via http and dumps it to stdout.
 */
#include "wvargs.h"
#include "wvhttp.h"
#include "wvfile.h"
#include "wvlog.h"
#include "strutils.h"
#include <signal.h>


int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    WvLog log("httptest", WvLog::Info);

    WvStringList extra_args;
    WvArgs args;
    args.add_required_arg("URL");

    if (!args.process(argc, argv, &extra_args))
        return 1;
    
    WvURL url(extra_args.popstr());

    WvHTTPStream http(url);
    http.autoforward(*wvcon);
    
    while (http.isok())
        http.runonce();
        
    if (!http.isok() && http.geterr())
	log("http: %s\n", http.errstr());

    return 0;
}
