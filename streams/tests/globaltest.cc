/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Tests that the return value of select() on a particular stream is
 * not affected by the readiness of the global list.
 */

#include "wvistreamlist.h"
#include "wvlog.h"

static WvLog log("globaltest");

void callback1(WvStream& s, void*)
{
    log("callback called for s1 (rearming alarm)\n");
    s.alarm(0);
}

void callback2(WvStream& s, void*)
{
    log("callback called for s2? weird...\n");
}

int main()
{
    WvStream s1;
    WvStream s2;

    s1.setcallback(callback1, 0);
    s2.setcallback(callback2, 0);

    s1.alarm(0);

    assert(s1.isok());
    assert(s2.isok());

    assert(s1.select(0));
    assert(!s2.select(0));

    WvIStreamList::globallist.append(&s1, false);

    assert(s1.isok());
    assert(s2.isok());

    assert(s1.select(0));
    assert(!s2.select(0));

    assert(WvIStreamList::globallist.select(0));

    log("test passed!\n");
}

