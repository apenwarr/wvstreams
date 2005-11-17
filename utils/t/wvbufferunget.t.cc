#include "wvbuf.h"
#include "wvstream.h"
#include "wvtest.h"
#include "wvfile.h"

WVTEST_MAIN("unget_with_multiple_buffers")
{
    WvDynBuf buffy;
    buffy.putstr("Test");
    buffy.getstr();
    buffy.unget(4);
    
    WvFile pass("/etc/passwd", O_RDONLY);
    pass.runonce();
    pass.read(buffy, 1024);
    
    unsigned int s = buffy.used();
    wvcon->print("Called buffy.getstr(%s)\n", s);
    buffy.getstr(s);
    wvcon->print("len:%s ungettable:%s\n",
            s, buffy.ungettable());
    WVFAIL(s != buffy.ungettable());
//    buffy.unget(s);
    WVPASS(1);
}

