#include "wvtest.h"
#include "wvlogfile.h"
#include "wvfileutils.h"

WVTEST_MAIN("wvlogfile")
{
    WvString name(wvtmpfilename("wvtest-log"));
    WvLogFile f(name);
    WvString fullname(f.start_log());
    
    WVFAILEQ(fullname, name);
    
    WvLog log("hello", WvLog::Info);
    WVPASS(true);
    
    WVPASS(access(fullname, R_OK) == 0);
    log.print("log test\n");
    WVPASS(true);
}
