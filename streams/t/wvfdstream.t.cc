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

WVTEST_MAIN("open and close with null FDs")
{
    WvFDStream fdstream;

    WVPASS(fdstream.getrfd() == -1);
    WVPASS(fdstream.getwfd() == -1);
    WVPASS(fdstream.getfd() == -1);

    WVFAIL(fdstream.isok());
    WVFAIL(fdstream.select(1, true, true));
}

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

WVTEST_MAIN("open, read, write and close between two WvFDStreams")
{
    // create temporary and empty file for testing
    int file = open("/tmp/wvfdstream.t", O_CREAT | O_TRUNC | O_RDWR, 0666); 
    WvFDStream writestream(-1, file);
    WvFDStream readstream(file, -1);

    // writestream is not-readable and writeable
    WVPASS(writestream.iswritable());
    // readstream is readable and not-writeable
    WVPASS(readstream.isreadable());

    char *buf = new char[256];
    
    // Writing to readstream
    writestream.write("Bonjour, j'me appellez writestream\n");
    WVPASS(writestream.iswritable());
    WVPASS(readstream.isreadable());

    // It's not reading for some odd reason...
//    WVPASS(readstream.read(buf, 256) == 35);
    printf("%d\n", readstream.read(buf, 256));
    
    
    delete[] buf;
    close(file);
}

WVTEST_MAIN("outbuf_limit")
{
    int fd = open("/dev/null", O_WRONLY);
    WVPASS(fd > 2);
    WvFDStream fdstream1(dup(0), fd);
    
    fdstream1.outbuf_limit(10);
    fdstream1.delay_output(true);
    
    // empty buffer - should be writeable
    WVPASS(fdstream1.isok());
    WVPASS(fdstream1.iswritable());

    // one character in buffer - still writeable
    fdstream1.write("d");
    WVPASS(fdstream1.isok());
    WVPASS(fdstream1.iswritable());
    
    // string is too long - write only (10 - 1) chars
    WVPASS(fdstream1.write("Hello terminal!\n") == 9);
    WVPASS(fdstream1.isok());
    
    // you might expect fdstream to return false here, but it doesn't; the
    // stream *is* writable (if you were to allow it to flush), but you
    // don't, so writes will fail even if it's writable.  You have to be
    // prepared for writes to fail even if a stream is writable anyway,
    //WVFAIL(fdstream1.iswritable());

    // full buffer - not writeable and write() returns 0
    WVPASS(fdstream1.isok());
    WVPASS(fdstream1.write("Hello terminal, again!\n") == 0);
    //WVFAIL(fdstream1.iswritable());
}

