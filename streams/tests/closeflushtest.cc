#include "wvstreamclone.h"
#include "wvfdstream.h"
#include "wvlog.h"

class SillyStream : public WvFDStream
{
public:
    int count;
    
    SillyStream() : WvFDStream(1)
    {
	count = 0;
    }
    
    virtual size_t uwrite(const void *buf, size_t size)
    {
	++count;
	fprintf(stderr, "uwrite #%d (%d bytes)\n", count, (int)size);
	if (count == 2)
	    close(); // pretend we had a socket error
	return 0;
    }
    
    virtual bool post_select(SelectInfo &si)
    {
	fprintf(stderr, "post_select(%d,%d)\n",
		si.wants.readable, si.wants.writable);
	return WvFDStream::post_select(si) || true;
    }
    
    virtual void close()
    {
	fprintf(stderr, "closing.\n");
	WvFDStream::close();
    }
    
    size_t obu()
    {
	return outbuf.used();
    }
};


int main()
{
    WvLog log("closeflushtest");
    
    log("Starting.\n");
    
    {
	SillyStream s;
	
	s.delay_output(true);
	s.write("Hello world\n");
	log("Said hello -- %s\n", s.obu());
	
	s.delay_output(false);
	log("Cancelled delay -- %s\n", s.obu());
	
	s.flush_then_close(10000);
	log("Scheduled close -- %s\n", s.obu());
	
	log("Select...\n");
	s.select(100);
	
	log("Destroying -- %s\n", s.obu());
    }
    
    log("Didn't crash.\n");
    return 0;
}
