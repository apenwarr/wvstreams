#include "wvtest.h"
#include "wvstreamclone.h"
#include "wvfdstream.h"
#include "wvlog.h"

#define TEST_IS_FULLY_CONVERTED 1
//FIXME: absolutely simple simple test right now, built from closeflushtest


#ifdef TEST_IS_FULLY_CONVERTED

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


WVTEST_MAIN("old-style")
{
//    WvLog log("closeflushtest");
    
//    log("Starting.\n");
    
    {
	SillyStream s;
	
	s.delay_output(true);
	s.write("Hello world\n");
//	log("Said hello -- %s\n", s.obu());
        WVPASS(s.obu() == 12);
	
	s.delay_output(false);
//	log("Cancelled delay -- %s\n", s.obu());
	WVPASS(s.obu() == 12);
	
	s.flush_then_close(10000);
//	log("Scheduled close -- %s\n", s.obu());
	WVPASS(s.obu() == 12);
        
//	log("Select...\n");
	s.select(100);
	
//	log("Destroying -- %s\n", s.obu());
	WVPASS(s.obu() == 0);
    }
//    log("Didn't crash.\n");
}
#else
#warning "Skipping old-style section of unittest until it is converted"
#endif
