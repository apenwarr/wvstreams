#include "wvrateadjust.h"
#include "wvtest.h"
#include "strutils.h"

#define MSEC_SLEEP 1000
#define SAMPSIZE 2
#define INRATE 100
#define OUTRATE 105

#define BLK (INRATE * SAMPSIZE * MSEC_SLEEP / 1000)


int main()
{
    assert((BLK % SAMPSIZE) == 0);
    
    WvDynBuf inbuf, outbuf, outbuf2;
    const void *ptr;
    char chunk[BLK];
    int n = 0;
    size_t total = 0, total2 = 0, used;
    WvTime epoch = wvtime();
    
    WvTest test;
    WvLog dump("Outbuf", WvLog::Debug2);
    WvRateAdjust adj(SAMPSIZE, OUTRATE), adj2(SAMPSIZE, &adj);
    
    for (int i = 0; i < (int)sizeof(chunk); i++)
	chunk[i] = (i+1) % 256;
    
    for (;;)
    {
	usleep(1000*MSEC_SLEEP);
	n = msecdiff(wvtime(), epoch);
	
	inbuf.put(chunk, sizeof(chunk));
	adj.encode(inbuf, outbuf);
	
	used = outbuf.used();
	total += used;
	
	test("FWD: blk=%s, out=%s (%s.%s/sec)\n",
	     sizeof(chunk), total,
	     total * 1000 / n, (total * 10000 / n) % 10);
	
	ptr = outbuf.get(used);
	
	//dump("%s\n", hexdump_buffer(chunk, sizeof(chunk)));
	dump("%s\n", hexdump_buffer(ptr, used < 64 ? used : 64));
	
	outbuf.unget(used);
	adj2.encode(outbuf, outbuf2);
	
	total2 += outbuf2.used();
	outbuf2.zap();
	
	test("REV: out=%s (%s.%s/sec)\n",
	     total2, total2 * 1000 / n, (total2 * 10000 / n) % 10);
    }
}
