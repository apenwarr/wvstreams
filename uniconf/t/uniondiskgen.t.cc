/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2006 Net Integration Technologies, Inc.
 * 
 * Unit tests for the UniConf on-disk generator.
 */ 

#include "wvautoconf.h"

#ifdef WITH_QDBM

#include "uniondiskgen.h"
#include "uniconf.h"
#include "uniconfroot.h"
#include "wvtest.h"
#include "wvlog.h"
#include "uniconfgen-sanitytest.h"
#include "uniconfdaemon.h"
#include "wvfork.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

WVTEST_SLOW_MAIN("UniOnDiskGen Sanity Test")
{
    WvString filename("/tmp/ondisk-%s.qdb", getpid());
    UniOnDiskGen *gen = new UniOnDiskGen(filename);
    UniConfGenSanityTester::sanity_test(gen, "ondisk:");
    WVRELEASE(gen);
}

WVTEST_MAIN("Recursive iteration")
{
    UniOnDiskGen *d = new UniOnDiskGen(WvString::null);
    UniConfRoot root;
    root.mountgen(d, false);

    root.xset("Foo/a", "3");
    root.xset("Foo/B", "4");
    root.xset("Foo/Boo", "7");
    root.xset("Foo/b/c", "5");
    root.xset("Foo/B/d", "6");
    root.xset("Bar/1", "1");

    UniConf::RecursiveIter ii(root["/"]);
    int count;
    for (count = 0, ii.rewind(); ii.next(); count++)
    {
        WVFAILEQ(ii->fullkey().printable(), root[ii->fullkey()].getme());
        // Skip autovivified entries
        if (ii->getmeint() != 0)
        {
            WVPASSEQ(ii->getmeint(), count);
            WVPASSEQ(root[ii->fullkey()].getmeint(), count);
        }
    }
    WVPASSEQ(count, 8);
}

WVTEST_MAIN("Iteration")
{
    UniOnDiskGen *d = new UniOnDiskGen(WvString::null);
    UniConfRoot root;
    root.mountgen(d, false);

    root.xset("Foo/a", "3");
    root.xset("Foo/B", "4");
    root.xset("Foo/Boo", "5");
    root.xset("Foo/b/c", "6");
    root.xset("Foo/B/d", "7");
    root.xset("Bar/1", "1");

    int count;

    UniConf::Iter ii(root["/"]);
    for (count = 0, ii.rewind(); ii.next(); count++)
    {
        WVFAILEQ(ii->fullkey().printable(), root[ii->fullkey()].getme());
        WVPASSEQ(ii->getme(), "");
    }
    WVPASSEQ(count, 2);

    int start = 3;
    UniConf::Iter jj(root["Foo"]);
    for (count = 0, jj.rewind(); jj.next(); count++)
    {
        WVFAILEQ(jj->fullkey().printable(), root[jj->fullkey()].getme());
        WVPASSEQ(jj->getmeint(), count + start);
        WVPASSEQ(root[jj->fullkey()].getmeint(), count + start);
    }
    WVPASSEQ(count, 3);
}
#endif
