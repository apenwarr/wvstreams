#include "wvtest.h"
#include "wvtimeutils.h"

#define private public
#define protected public
#include "wvstream.h"


WVTEST_MAIN()
{
    WvStream s;
    char buf[1024];
    
    // buffered reads and writes
    WVPASS(!s.isreadable());
    WVPASS(!s.iswritable());
    WVFAIL(s.read(buf, 1024) != 0);
    WVPASS(s.write(buf, 1024) == 1024);
    WVPASS(!s.iswritable());
    WVPASS(!s.isreadable());
    WVPASS(s.isok());
    
    // close() shouldn't have to wait to flush buffers, because plain
    // WvStream has no way to actually flush them.
    WvTime t1 = wvtime();
    s.close();
    WvTime t2 = wvtime();
    WVPASS(msecdiff(t2, t1) >= 0);
    WVPASS(msecdiff(t2, t1) < 1000);
	
    // after close()
    WVPASS(!s.isok());
}


WVTEST_MAIN()
{
    WvStream s;
    char buf[1024];

    // noread/nowrite behaviour
    s.nowrite();
    WVPASS(s.isok());
    WVFAIL(s.write(buf, 1024) != 0);
    s.noread();
    WVPASS(!s.isok());
}


WVTEST_MAIN()
{
    WvStream s;
    char buf[1024];
    
    WVPASS(!s.isreadable());
    s.inbuf.putstr("a\n b \r\nline");
    WVPASS(s.isreadable());
    s.noread();
    WVPASS(s.isreadable());
    
    WVPASS(s.read(buf, 2) == 2);
    char *line = s.getline(0);
    WVPASS(line);
    WVPASS(line && !strcmp(line, " b \r"));
    line = s.getline(0);
    WVPASS(line);
    WVPASS(line && !strcmp(line, "line"));
    WVPASS(!s.getline(0));
    
    WvTime t1 = wvtime();
    WVPASS(!s.getline(500));
    WvTime t2 = wvtime();
    WVPASS(msecdiff(t2, t1) >= 0);
    WVPASS(msecdiff(t2, t1) < 400); // noread().  shouldn't actually wait!
    
    // FIXME: avoid aborting the entire test here on a freezeup!
    ::alarm(5); // crash after 5 seconds
    WVPASS(!s.getline(-1));
    ::alarm(0);
}


// FIXME: lots of remaining untested stuff
//    callback / closecallback
//    autoforward/noautoforward
//    continue_select / continue_read
//    seterr
//    read_requires_writable / write_requires_readable
//    flush_then_close
//    alarm() and alarm_remaining()
//    delay_output()
//    drain()
//    queuemin()
//    select including globallist (runonce())
//        force_select
//    print() with a format
//    src()
//    
// Wow, jbrown was right.  This *is* a pretty tangled mess!
