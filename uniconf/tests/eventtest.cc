/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the UniConf default event handler system.
 */
#include "unievents.h"
#include "uniconfini.h"
#include "wvtest.h"

int main()
{
    WvTest test;
    UniConf h;
    UniConfNotifier notifier(h);
    UniConfEvents ev1(h, "ev1"), ev2(h["spoon"], "ev2");
    bool b1 = false, b2 = false, b3 = false, b4 = false;
    
    ev1.add_setbool(&b1, "foo/blah/weasels");
    ev1.add_setbool(&b2, "*/blah/*");
    ev1.add_setbool(&b3, "*/*/weasels");
    ev2.add_setbool(&b4, "*/weasels");
    
    notifier.run(); test("bools: %s/%s/%s/%s\n", b1, b2, b3, b4);
    
    h["/foo/blah/neat/y"] = 5;
    notifier.run(); test("bools: %s/%s/%s/%s\n", b1, b2, b3, b4);

    h["/fork/noodle/weaselsy"] = 6;
    notifier.run(); test("bools: %s/%s/%s/%s\n", b1, b2, b3, b4);

    b1 = b2 = b3 = b4 = false;
    h["/spoon/nosed/weasels"] = 7;
    notifier.run(); test("bools: %s/%s/%s/%s\n", b1, b2, b3, b4);
    
    h["/foo/blah/weasels"] = 9;
    notifier.run(); test("bools: %s/%s/%s/%s\n", b1, b2, b3, b4);
    
    h.dump(*wvcon, true);
}
