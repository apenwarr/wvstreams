#include "wvtcp.h"
#include "wvtest.h"
#include <errno.h>

WVTEST_MAIN("tcp connection")
{
    WvTCPListener listen("0.0.0.0:0");
    WvIPPortAddr port(*listen.src());
    WvTCPConn tcp(port);
    tcp.write("foo\n");
    
    listen.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    listen.runonce(100);
    WvTCPConn *in = listen.accept();
    WVPASS(in);
    if (!in) return; // forget it...
    
    WVPASS(in->isok());
    
    tcp.runonce(100);
    tcp.runonce(100);
    tcp.runonce(100);
    
    WVPASS(in->isreadable());
    
    WvString line = in->blocking_getline(100);
    WVPASSEQ(line, "foo");
    
    in->release();
}


WVTEST_MAIN("connection refused")
{
    WvTCPListener listen("0.0.0.0:0");
    WvIPPortAddr port(*listen.src());
    listen.close();
    
    WvTCPConn tcp(port);
    listen.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    tcp.runonce(0);
    
    WVFAIL(tcp.isok());
    WVPASSEQ(tcp.geterr(), ECONNREFUSED);
    WVPASSEQ(tcp.errstr(), strerror(ECONNREFUSED));
    
    tcp.write("foo\n");
    WVFAIL(tcp.isok());
    WVPASSEQ(tcp.geterr(), ECONNREFUSED);
    WVPASSEQ(tcp.errstr(), strerror(ECONNREFUSED));
}
