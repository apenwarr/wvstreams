/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the WvHConf default event handler system.
 */
#include "wvhconfevents.h"
#include "wvhconfini.h"

int main()
{
    WvLog log("eventtest", WvLog::Info);
    WvHConf h;
    WvHConfEvents ev(h);
    bool b1 = false, b2 = false, b3 = false;
    
    ev.add_setbool(&b1, "/foo/blah/weasels");
    ev.add_setbool(&b2, "/*/blah/*");
    ev.add_setbool(&b3, "/*/*/weasels");
    
    ev.do_callbacks(); log("bools: %s/%s/%s\n", b1, b2, b3);
    
    h["/foo/blah/neat/y"] = 5;
    ev.do_callbacks(); log("bools: %s/%s/%s\n", b1, b2, b3);

    h["/fork/noodle/weaselsy"] = 6;
    ev.do_callbacks(); log("bools: %s/%s/%s\n", b1, b2, b3);

    h["/spoon/nosed/weasels"] = 7;
    ev.do_callbacks(); log("bools: %s/%s/%s\n", b1, b2, b3);
    
    h["/foo/blah/weasels"] = 9;
    ev.do_callbacks(); log("bools: %s/%s/%s\n", b1, b2, b3);
    
    h.dump(*wvcon, true);
}
