/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * One more attempt at making a decent stream for Linux /dev/dsp.  This is
 * apparently much trickier than it looks.
 */
#include "wvdsp.h"
#include <wvlogrcv.h>

void usage()
{
    wvcon->print("dsptest [-r] [-o] [-l latency] [-d]\n");
    exit(1);
}

int main(int argc, char **argv)
{
    WvLog log("dsptest", WvLog::Info);
    
    bool realtime = false;
    bool oss = false;
    int latency = 0;
    int debuglvl = 0;
    
    int c;
    opterr = true;
    while ((c = getopt(argc, argv, "rol:dDh?")) >= 0)
    {
        switch (c)
        {
            case 'r':
                realtime = true;
                break;
            case 'o':
                oss = true;
                break;
            case 'l':
                latency = WvString(optarg).num();
                break;
            case 'd':
                debuglvl = 1;
                break;
            case 'D':
                debuglvl = 2;
                break;
            case 'h':
            case '?':
            default:
                usage();
                break;
        }
    }

    WvLog::LogLevel loglvl = WvLog::Info;
    switch (debuglvl)
    {
        case 0: loglvl = WvLog::Info; break;
        case 1: loglvl = WvLog::Debug; break;
        case 2: loglvl = WvLog::Debug2; break;
    };
    WvLogConsole clog(1, loglvl);

    log("Opening dsp...");
    WvDsp dsp(latency, 44100, 16, 2, true, true, realtime, oss);
    log("done.\n");
    
    if (!dsp.isok())
    {
	log("dsp: %s\n", dsp.errstr());
	return 1;
    }
    
    //dsp.autoforward(dsp);
    
    char buf[10240];
    size_t len;
    
    log("is:%s os:%s\n", dsp.ispace(), dsp.ospace());
    //sleep(1);
    
    memset(buf, 0, sizeof(buf));
    //dsp.write(buf, 0);
    //dsp.write(buf, sizeof(buf));
    //dsp.write(buf, sizeof(buf));
    
    while (dsp.isok())
    {
	if (dsp.select(-1))
	{
	    len = dsp.read(buf, sizeof(buf));
	    //if (len)
	    //log("[%s]", len);
	    if (!dsp.isok())
		break;
	    if (len)
		dsp.write(buf, len);
	}
    }
    
    if (!dsp.isok())
	log("exiting: dsp not okay: %s\n", dsp.errstr());
    
    return 0;
}
