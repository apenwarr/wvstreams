/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvPipe test.  Allows you to enter bash commands, runs them, and pipes the
 * output back to you.
 */

#include <wvtest.h>

#include <wvpipe.h>
#include <wvlog.h>
#include <wvstringlist.h>
#include <wvbufbase.h>

WVTEST_MAIN ("WvPipe with environment")
{
    
    WvDynBuf foo;
    WvStringList envs;
    envs.append("FOOBAR=foobar");
    
    
    const char *av[] = 
    {
	"/bin/bash",
	NULL
    };
    
    WvPipe p(av[0], av, true, true, false, 0, 1, 2, &envs);
    
//    wvcon->autoforward(p);
//    p.autoforward(*wvcon);

    p.write("echo $FOOBAR\n");
    if (p.select(100)) 
    {
        p.read(foo, 1024);
        WvString isfoo(foo.getstr());
        WVPASS(isfoo == "foobar\n");
    }
}
