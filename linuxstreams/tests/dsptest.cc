/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * One more attempt at making a decent stream for Linux /dev/dsp.  This is
 * apparently much trickier than it looks.
 */
#include "wvdsp.h"

int main()
{
    WvLog log("dsptest", WvLog::Info);
    
    log("Opening dsp...");
    WvDsp dsp(0, 44100, 16, 2, true, true);
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
