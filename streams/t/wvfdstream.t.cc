#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>

#include "wvfdstream.h"
#include "wvfile.h"
#include "wvfileutils.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvsocketpair.h"
#include "wvstreamclone.h"
#include "wvtest.h"
#include "wvfileutils.h"

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

WVTEST_MAIN("stdout clone")
{
    {
	WvFDStream s(dup(1));
	WVPASS(s.isok());
	WVPASSEQ(s.geterr(), 0);
	s.print("This is stdout!\n");
	WVPASS(s.isok());
	WVPASSEQ(s.geterr(), 0);
	s.close();
	WVFAIL(s.isok());
	WVPASSEQ(s.geterr(), 0);
    }
    
    {
	int fd = dup(1);
	WvFDStream s(fd);
	WVPASS(s.isok());
	WVPASSEQ(s.geterr(), 0);
	s.print("Boink!\n");
	WVPASS(s.isok());
	WVPASSEQ(s.geterr(), 0);
	close(fd);
	s.print("this will write to an invalid fd\n");
	WVFAIL(s.isok());
	WVPASSEQ(s.geterr(), EBADF);
    }
}

WVTEST_MAIN("open, read, write and close between two WvFDStreams")
{
    ::unlink("wvfdstream.t.tmp");
    
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
    WVPASSEQ(writestream.write("Bonjour! Je m'appelle writestream"), 33);
    WVPASS(writestream.iswritable());
    WVPASS(readstream.isreadable());

    char *buf = new char[256];
    memset(buf, 0, 256);
    
    // Reading from file
    WVPASS(writestream.select(0, false, true));
    WVPASS(1);
    WvString line(readstream.blocking_getline(-1));
    WVPASSEQ(line, "Bonjour, je m'appelle writestream");
    WVPASS(readstream.isreadable());
    
    WVPASSEQ(readstream.read(buf, 256), 33);
    // read() is not supposed to insert the null terminator at the end of
    // the char string, so do it manually
    buf[33] = '\0';
    WVPASSEQ(buf, "Bonjour! Je m'appelle writestream");
   
    deletev buf;
    close(file1);
    close(file2);
}

WVTEST_MAIN("outbuf_limit")
{
    int fd = open("wvfdstream.t.tmp", O_WRONLY);
    printf("Trying to open wvfdstream.t.tmp to write\n");
    if (!WVPASS(fd > 2))
    {
        printf("(fd==%d) Are you sure we can write to wvfdstream.t.tmp?\n",
	       fd);
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


static void myclosecb(int *i, WvStream &s)
{
    (*i)++;
}

WVTEST_MAIN("closecallback")
{
    WvFdStream s(dup(0), dup(1));
    int i = 0;
    s.setclosecallback(
	       WvBoundCallback<IWvStreamCallback,int*>(&myclosecb, &i));
    
    WVPASS(s.isok());
    s.nowrite();
    WVPASS(s.isok());
    WVPASSEQ(i, 0);
    s.noread();
    s.runonce(0);
    WVFAIL(s.isok());
    WVPASSEQ(i, 1);
}


WVTEST_MAIN("inbuf after read error")
{
    int socks[2];
    WVPASS(!wvsocketpair(SOCK_STREAM, socks));
    WvFdStream s1(socks[0]), s2(socks[1]);
    s1.print("1\n2\n3\n4\n");
    WVPASSEQ(s2.blocking_getline(1000, '\n', 1024), "1");
    s1.close();
    WVPASSEQ(s2.blocking_getline(1000, '\n', 1024), "2");
    WVPASSEQ(s2.blocking_getline(1000, '\n', 1024), "3");
    WVPASS(s2.isok());
    WVPASSEQ(s2.blocking_getline(1000, '\n', 1024), "4");
    WVFAIL(s2.blocking_getline(1000, '\n', 1024));
    WVFAIL(s2.isok());
}


class FooFD : public WvFDStream {
public:
    FooFD(int fd) : WvFDStream(fd)
    {
	WVPASS(isreadable());
        called = false;
        setcallback(WvStreamCallback(this,
        	&FooFD::fooback), NULL);
        WvIStreamList::globallist.append(this, false, "FooFD");
    }

    ~FooFD()
    {
	WvIStreamList::globallist.unlink(this);
    }

    void fooback(WvStream &, void *)
    {
	called = true;
    }

    bool called;
};

WVTEST_MAIN("Test undo_force_select() on a WvFDStream")
{
    // create our test file
    WvString testfile = wvtmpfilename("wvfdstream-testfile-");
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
    WVPASS(foof.isok());
    WVPASS(foof.select(0));
    WVPASS(WvIStreamList::globallist.select(0));
    WVPASS(foof.called);

    foof.called = false;
    WVPASS(!foof.called);

    // undo_force_select() and make sure we're not called
    foof.undo_force_select(true, true, true);

    // can't use runonce() here because it should be false
    if (WvIStreamList::globallist.select(0))
	WvIStreamList::globallist.callback();

    WVPASS(!foof.called);

    unlink(testfile);
}

