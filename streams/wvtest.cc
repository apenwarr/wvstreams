/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * The beginnings of an automated testing framework.  See wvtest.h.
 */
#include "wvtest.h"
#include <time.h>

WvTestRcv::WvTestRcv() : WvLogConsole(1, WvLog::NUM_LOGLEVELS)
{
    testnum = 0;
    time_t now = time(NULL);
    
    print("\nWvTest BEGIN: %s", ctime(&now));
}


WvTestRcv::~WvTestRcv()
{
    time_t now = time(NULL);
    print("WvTest END: %s\n", ctime(&now));
}


void WvTestRcv::_begin_line()
{
    if (!strcmp(appname(last_source), "WvTest"))
    {
	prefix = WvString("WvTest #%s: ", ++testnum);
	testing = true;
    }
    else
    {
	prefix = WvString("  %s: ", appname(last_source));
	testing = false;
    }
    prelen = strlen(prefix);
    
    _mid_line(prefix, prelen);
}


void WvTestRcv::_end_line()
{
    // insert an extra newline after an actual test line
    //if (testing)
    //   _mid_line("\n", 1);
    testing = false;
}


WvTest::WvTest() : WvLog("WvTest", WvLog::Info)
{
    // nothing special
}


WvTest::~WvTest()
{
    // nothing special
}
