#include "wvtest.h"
#include "wvlogbuffer.h"
#include "wvlog.h"

void loganddelete()
{
    WvLog loggy ("temporary log");
    loggy.uwrite("msg", 3);
} // loggy gets deleted when this returns

WVTEST_MAIN("test for accessing free'd memory in wvlogbuffer")
{
    // Create wvlogbuffer to recieve messages
    WvLogBuffer *logbuf = new WvLogBuffer(15, WvLog::Debug3);
    loganddelete();
    
    // Allocate a big string to overwrite the deleted WvLog with junk
    WvString a("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
               "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    logbuf->end_line();
    WVPASS(1); // If you get this far, you win!
}
