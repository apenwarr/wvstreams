/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Tests that the return value of select() on a particular stream is
 * not affected by the readiness of the global list.
 */

#include "wvistreamlist.h"
#include "wvlog.h"

static WvLog mylog("globaltest");
static int count = 0;

void callback1(WvStream& s)
{
    ++count;
    mylog("callback called for s1 (rearming alarm)\n");
    s.alarm(0);
}

void callback2()
{
    mylog("callback called for s2? weird...\n");
}

int main()
{
    WvStream s1;
    WvStream s2;

    s1.setcallback(wv::bind(callback1, wv::ref(s1)));
    s2.setcallback(callback2);

    s1.alarm(0);

    assert(s1.isok());
    assert(s2.isok());

    assert(s1.select(0));
    assert(!s2.select(0));

    WvIStreamList::globallist.append(&s1, false, "s1");

    assert(s1.isok());
    assert(s2.isok());

    assert(s1.select(0));
    assert(!s2.select(0));
    assert(WvIStreamList::globallist.select(0));

    assert(count == 3);

    mylog("test passed!\n");
}

