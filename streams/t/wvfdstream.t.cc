#include "wvtest.h"
#include "wvstreamclone.h"
#include "wvfdstream.h"
#include "wvlog.h"

//FIXME: absolutely simple simple test right now, built from closeflushtest
// BEGIN closeflushtest.cc definition
class SillyStream : public WvFDStream
{
public:
    int count;
    
    SillyStream() : WvFDStream(dup(1))
    {
	count = 0;
    }
    
    virtual size_t uwrite(const void *buf, size_t size)
    {
	++count;
	//	    fprintf(stderr, "uwrite #%d (%d bytes)\n", count, size);    
	if (count == 2)
	    close(); // pretend we had a socket error
	return 0;
    }
    
    virtual bool post_select(SelectInfo &si)
    {
	//	    fprintf(stderr, "post_select(%d,%d)\n",
	//	        si.wants.readable, si.wants.writable);
	return WvFDStream::post_select(si) || true;
    }
    
    virtual void close()
    {
	//      fprintf(stderr, "closing.\n");
	WvFDStream::close();
    }
    
    size_t obu()
    {
        return outbuf.used();
    }
};
// END closeflushtest.cc definition

WVTEST_MAIN("closeflushtest.cc")
{
    {
	SillyStream s;
	
	s.delay_output(true);
	s.write("Hello world\n");
        WVPASS(s.obu() == 12);
	
	s.delay_output(false);
	WVPASS(s.obu() == 12);
	
	s.flush_then_close(10000);
	WVPASS(s.obu() == 12);
        
	s.select(100);
	
	WVPASS(s.obu() == 0);
    }
}
