#include "wvdsp.h"
#include "wvlog.h"
#include "wvtimeutils.h"

#define RATE (8192)
#define BITS (16)
#define STEREO (false)
#define CHANNELS (STEREO ? 2 : 1)

#define ONESEC (RATE * BITS/8 * CHANNELS)
#define DELAY (ONESEC * 2)


long avg(const unsigned char *iptr, size_t len)
{
    long total = 0;
    
    for (unsigned i = 0; i < len; i++)
	total += iptr[i];
    
    return total/len;
}


long range(const unsigned char *iptr, size_t len)
{
    long min = 255, max = 0;
    
    for (unsigned i = 0; i < len; i++)
    {
	if (iptr[i] < min) min = iptr[i];
	if (iptr[i] > max) max = iptr[i];
    }
    
    return max - min;
}


long power(const unsigned char *iptr, size_t len, long avg)
{
    long total = 0;
    
    for (unsigned i = 0; i < len; i++)
	total += (iptr[i]-127) * (iptr[i]-127);
    
    return total/len; 
}


int main(int argc, char **argv)
{
    WvLog test("dspechotest", WvLog::Info);
    WvLog log("x", WvLog::Info);
    
    log("Opening dsp...");
    WvDsp dsp(200, RATE, BITS, STEREO, true, true, false, false);
    log("done.\n");
    
    if (!dsp.isok())
    {
	log("dsp: %s\n", dsp.errstr());
	return 1;
    }
    
    WvDynBuf buf, buf2;
    
    unsigned char *cptr;
    const unsigned char *iptr;
    size_t len;
    struct timeval t1 = wvtime(), t2;
    
    while (dsp.isok())
    {
	if (dsp.select(-1))
	{
	    cptr = buf.alloc(10240);
	    len = dsp.read(cptr, 10240);
	    buf2.put(cptr, len);
	    buf.unalloc(10240 - len);
	    
	    // only write data beyond the buffer fill minimum
	    if (buf.used() >= DELAY)
	    {
		len = buf.used() - DELAY;
		iptr = buf.get(len);
		dsp.write(iptr, len);
	    }
	}
	
	// do statistics once per second
	if (buf2.used() >= ONESEC)
	{
	    len = ONESEC;
	    iptr = buf2.get(len);
	    
	    t2 = wvtime();
	    long av = avg(iptr, len);
	    log("%s (%s) in %sms, avg=%s, range=%s, power=%s\n",
		len, buf2.used(), msecdiff(t2, t1),
		av, range(iptr, len), power(iptr, len, av));
	    t1 = t2;
	}
    }
    
    if (!dsp.isok())
	log("exiting: dsp not okay: %s\n", dsp.errstr());
    
    return 0;
}
