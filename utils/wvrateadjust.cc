/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvRateAdjust is a WvEncoder that makes sure data comes out of it at a
 * given average rate.
 * 
 * See wvrateadjust.h.
 */
#include "wvrateadjust.h"

WvRateAdjust::WvRateAdjust(int _sampsize, int _orate)
    : log("RateAdj", WvLog::Debug5)
{
    orate_n = _orate;
    orate_d = 1;
    match_rate = NULL;
    
    init(_sampsize);
}


WvRateAdjust::WvRateAdjust(int _sampsize, WvRateAdjust *_match_rate)
    : log("RateAdj", WvLog::Debug5)
{
    match_rate = _match_rate;
    assert(match_rate);
    
    orate_n = match_rate->irate_n;
    orate_d = match_rate->irate_d;
    
    init(_sampsize);
}


void WvRateAdjust::init(int _sampsize)
{
    sampsize = _sampsize;
    irate_n = 1;
    irate_d = 1;
    epoch.tv_sec = 0;
    bucket = 0;
}


// we always use all input samples and produce an appropriate number of
// output samples.
bool WvRateAdjust::_encode(WvBuf &inbuf, WvBuf &outbuf, bool flush)
{
    if (!inbuf.used()) return true;
    assert((inbuf.used() % sampsize) == 0); // can't deal with partial samples
    
    WvTime now = wvtime();
    unsigned isamps = inbuf.used() / sampsize;
    
    // match our output rate to another stream's input rate, if requested
    if (match_rate)
    {
	orate_n = match_rate->irate_n;
	orate_d = match_rate->irate_d;
    }
    
    // adjust the input rate estimate
    if (!epoch.tv_sec)
	epoch = now;
    irate_n += isamps * 10;
    irate_d = msecdiff(wvtime(), epoch) / 100;
    if (!irate_d)
	irate_d = 1;
    
    log("irate=%s (%s/%s), orate=%s (%s/%s), bucket=%s\n",
	getirate(), irate_n, irate_d, getorate(), orate_n, orate_d,
	bucket);
    
    // reduce the rate estimate if it's getting out of control FIXME:
    // this method is (almost) unbearably cheesy because it's very
    // "blocky" - it doesn't happen every time, so it'll cause sudden
    // jumps from one value to the next.  Hopefully not a big deal,
    // since the input rate is supposed to be constant anyway.  The
    // hardcoded constants are also rather weird.
    if (irate_d > 100) // ten seconds
    {
	epoch.tv_sec++; // time now starts one second later
	irate_n = irate_n * (irate_d - 10)/irate_d;
	irate_d -= 10;
	
	log("  JUMP!  new irate=%s (%s/%s)\n", getirate(), irate_n, irate_d);
    }
	
    int plus = orate_n * irate_d, minus = irate_n * orate_d;
    log("plus=%s, minus=%s, ", plus, minus);

    unsigned omax = isamps + isamps/2;
    log("isamps=%s, omax=%s\n", isamps, omax);
    
    const unsigned char *iptr = inbuf.get(isamps * sampsize);
    unsigned char *ostart, *optr;
    
    ostart = optr = outbuf.alloc(omax * sampsize);

    // copy the buffers using the "Bresenham line-drawing" algorithm.
    for (unsigned s = 0; s < isamps; s++, iptr += sampsize)
    {
	bucket += plus;
	//log("s=%s, bucket=%s (+%s, -%s)\n", s, bucket, plus, minus);
	
	while (bucket >= minus)
	{
	    // allocate more buffer space if needed
	    if ((unsigned)(optr - ostart) >= omax * sampsize)
		ostart = optr = outbuf.alloc(omax * sampsize);
	    
	    for (int i = 0; i < sampsize; i++)
		optr[i] = iptr[i];
	    optr += sampsize;
	    bucket -= minus;
	}
    }
    
    unsigned un = omax*sampsize - (optr - ostart);
    log("unalloc %s/%s (%s)\n", un, omax*sampsize, optr-ostart);
    outbuf.unalloc(un);
    
    return true;
}


