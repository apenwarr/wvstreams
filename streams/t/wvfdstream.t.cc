#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "wvtest.h"
#include "wvstreamclone.h"
#include "wvfdstream.h"
#include "wvlog.h"
#include "wvfile.h"
#include "wvistreamlist.h"

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
    WVFAIL(fdstream.select(1, true, false));
    WVFAIL(fdstream.select(1, false, true));
    WVFAIL(fdstream.select(1, true, true));

    WVFAIL(fdstream.iswritable());
    WVFAIL(fdstream.isreadable());
}

WVTEST_MAIN("open, read, write and close between two WvFDStreams")
{
    // create temporary and empty file for testing
    printf("Trying to open wvfdstream.t.tmp to write\n");
    int file1 = open("wvfdstream.t.tmp", O_CREAT | O_TRUNC | O_WRONLY, 0666); 
    if (!WVPASS(file1 >= 0))
        printf("Are you sure we can write to wvfdstream.t.tmp?\n");
    printf("Trying to open wvfdstream.t.tmp to read\n");
    int file2 = open("wvfdstream.t.tmp", O_CREAT | O_TRUNC | O_RDONLY, 0666); 
    if (!WVPASS(file2 >= 0))
        printf("Are you sure we can read from wvfdstream.t.tmp?\n");
    
    WvFDStream writestream(-1, file1);
    WvFDStream readstream(file2, -1);

    // writestream is not-readable and writeable
    WVPASS(writestream.iswritable());
    WVFAIL(writestream.isreadable());
    // readstream is readable and not-writeable
    WVPASS(readstream.isreadable());
    WVFAIL(readstream.iswritable());

    // Writing to file
    WVPASSEQ(writestream.write("Bonjour, je m'appelle writestream\n"), 34);
    WVPASSEQ(writestream.write("Bonjour, je m'appelle writestream"), 33);
    WVPASS(writestream.iswritable());
    WVPASS(readstream.isreadable());

    char *buf = new char[256];
    memset(buf, 0, 256);
    
    // Reading from file
    writestream.select(0, false, true);
    WvString line(readstream.getline(-1));
    WVPASSEQ(line, "Bonjour, je m'appelle writestream");
    
    WVPASSEQ(readstream.read(buf, 256), 33);
    // read() is not supposed to insert the null terminator at the end of the char string, so do it manually
    buf[33] = '\0';
    WVPASS(strcmp((const char*)buf, "Bonjour, je m'appelle writestream") == 0);
   
    deletev buf;
    close(file1);
    close(file2);
}

WVTEST_MAIN("outbuf_limit")
{
    int fd = open("/dev/null", O_WRONLY);
    printf("Trying to open wvfdstream.t.tmp to read/write\n");
    if(!WVPASS(fd > 2))
    {
        printf("Are you sure we can write to wvfdstream.t.tmp?\n");
    }
    WvFDStream fdstream1(dup(0), fd);
    
    fdstream1.outbuf_limit(10);
    fdstream1.delay_output(true); // call flush explicitly
    
    // empty buffer
    WVPASS(fdstream1.isok());
    WVPASS(fdstream1.iswritable());

    // one character in buffer
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
    WVPASS(fdstream1.iswritable());

    // full buffer - write() returns 0 (i.e. it fails);
    WVPASS(fdstream1.isok());
    WVPASS(fdstream1.write("Hello terminal, again!\n") == 0);
    WVPASS(fdstream1.iswritable());
}

class FooFD : public WvStream {
public:
    FooFD(int fd) : y(fd)
    {
        called = false;
        y.setcallback(WvStreamCallback(this,
        	&FooFD::fooback), NULL);
        WvIStreamList::globallist.append(&y, false);
    }

    ~FooFD()
    {
	WvIStreamList::globallist.unlink(&y);
    }

    void fooback(WvStream &, void *)
    {
	called = true;
    }

    WvFDStream y;
    bool called;
};

WVTEST_MAIN("Test undo_force_select() on a WvFDStream")
{
    // create our test file
    WvString testfile("/tmp/wvfdstream-testfile-%s", getpid());
    WvFile x(testfile, O_CREAT|O_RDWR, 0666);
    x.print("moo\n");
    x.print("mow\n");
    x.print("maw\n");
    x.close();

    int fd = open(testfile, O_RDONLY);
    WVPASS(fd != -1);

    WvFDStream f(fd);
    WVPASS(f.isok());

    // should have some data for reading
    WVPASS(f.select(0));

    // we don't want to select on anything anymore
    f.undo_force_select(true, true, true);
    WVFAIL(f.select(0));

    // we want to continue selecting
    f.force_select(true, true, true);
    WVPASS(f.select(0));

    FooFD foof(fd);
    WVPASS(!foof.called);

    // check that our callback is called
    WvIStreamList::globallist.runonce();
    WVPASS(foof.called);

    foof.called = false;
    WVPASS(!foof.called);

// This fails, but uncommenting the select inside the loop
// makes it succeed properly
#if 0
    // undo_force_select() and make sure we're not called
    foof.y.undo_force_select(true, true, true);
    if (WvIStreamList::globallist.select(1000))
    {
//        printf("select(0) returned '%d'\n", foof.y.select(0));
	WvIStreamList::globallist.callback();
    }
    WVPASS(!foof.called);
#endif
}
