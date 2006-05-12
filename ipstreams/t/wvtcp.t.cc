#include "wvtcp.h"
#include "wvtest.h"
#include <errno.h>

WVTEST_MAIN("tcp connection")
{
    WvTCPListener listen("0.0.0.0:0");
    WVPASS(listen.isok());
    WvIPPortAddr port(*listen.src());
    //wvcon->print("Local port is '%s'\n", port);

    WvTCPConn tcp(port);
    WVPASS(1);
    tcp.write("foo\n");
    WVPASS(2);
 
    listen.runonce(0);
    WVPASS(3);
    tcp.runonce(0);
    WVPASS(4);
    tcp.runonce(0);
    WVPASS(5);
    tcp.runonce(0);
    tcp.runonce(0);
    WVPASS(6);
    listen.runonce(100);
    WVPASS(7);
    WvTCPConn *in = listen.accept();
    WVPASS(8);
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
    char buf[1024];
    size_t len;
    
    WvTCPListener listen("0.0.0.0:0");
    WVPASS(listen.isok());
    WvIPPortAddr port(*listen.src());
//    WvIPPortAddr port("127.0.0.1:11223");
//    WvIPPortAddr port("192.168.12.100:11223");
    //wvcon->print("Local port is '%s'\n", port);
    listen.close();
    listen.runonce(0);
    WVFAIL(listen.isok());
    WVPASSEQ(listen.geterr(), 0);
    
    WvTCPConn tcp(port);
    tcp.runonce(10000);
    tcp.runonce(10000);
    tcp.runonce(10000);
    tcp.runonce(10000);
    WVPASS(tcp.isconnected()); // even if error, we're past connection phase
    WVFAIL(tcp.isok());
    WVPASSEQ(tcp.geterr(), ECONNREFUSED);
    printf("Error string is '%s'\n", tcp.errstr().cstr());
//    WVPASSEQ(tcp.errstr(), strerror(ECONNREFUSED)); // wrong in win32
    
    len = tcp.read(buf, sizeof(buf));
    WVPASSEQ(len, 0);
    WVFAIL(tcp.isok());
    WVPASSEQ(tcp.geterr(), ECONNREFUSED);
    printf("Error string is '%s'\n", tcp.errstr().cstr());
//    WVPASSEQ(tcp.errstr(), strerror(ECONNREFUSED)); // wrong in win32
    
    tcp.write("foo\n");
    WVFAIL(tcp.isok());
    WVPASSEQ(tcp.geterr(), ECONNREFUSED);
    printf("Error string is '%s'\n", tcp.errstr().cstr());
//    WVPASSEQ(tcp.errstr(), strerror(ECONNREFUSED)); // wrong in win32
}
