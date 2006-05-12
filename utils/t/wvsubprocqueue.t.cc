/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A class for running a series or set of processes, one at a time.
 * See wvsubprocqueue.h.
 */
#include "wvsubprocqueue.h"
#include "wvtest.h"
#include "wvfile.h"

#include <unistd.h>

struct WvSubProcQueueTester
{
    WvString fn;
    WvString cmd1, cmd2, cmd2s;
    const char *argv1[4];
    const char *argv2[4];
    const char *argv2s[4];
    const char *argv3[4];
    int c1, c2; // cookies that we can point to - value doesn't matter

    WvSubProcQueueTester() : 
        fn("test-%s.tmp", getpid()),
        cmd1("echo cmd1 >>%s", fn), 
        cmd2("echo cmd2 >>%s", fn),
        cmd2s("sleep 1; %s", cmd2)
    { 
        argv1[0] = "sh"; argv1[1] = "-c", argv1[2] = cmd1, argv1[3] = NULL;
        argv2[0] = "sh"; argv2[1] = "-c", argv2[2] = cmd2, argv2[3] = NULL;
        argv2s[0]= "sh"; argv2s[1]= "-c", argv2s[2]= cmd2s, argv2s[3]= NULL;
        argv3[0] = "rm"; argv3[1] = "-f", argv3[2] = fn, argv3[3] = NULL;
    }

};

static WvString contents(WvStringParm fn)
{
    return WvFile(fn, O_RDONLY).blocking_getline(-1, 0);
}


static bool exists(WvStringParm fn)
{
    return !access(fn, F_OK);
}


WVTEST_MAIN("wvsubprocqueue1")
{
    WvSubProcQueueTester t;
    ::unlink(t.fn);
    WVFAIL(exists(t.fn));

    WvSubProcQueue q(1);
    
    // basic sequencing
    ::unlink(t.fn);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(NULL, t.argv3[0], t.argv3);
    q.add(NULL, t.argv2[0], t.argv2);
    WVPASSEQ(q.remaining(), 3);
    q.go();
    WVPASSEQ(q.running(), 1);
    WVPASSEQ(q.remaining(), 3); // one running, two waiting
    q.finish();
    WVPASSEQ(q.remaining(), 0);
    WVPASS(q.isempty());
    WVPASS(exists(t.fn));
    WVPASSEQ(contents(t.fn), "cmd2\n");
    
    // cookie sequencing and duplicate detection
    ::unlink(t.fn);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.add(NULL, t.argv2[0], t.argv2);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.add(NULL, t.argv2[0], t.argv2);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.finish();
    WVPASSEQ(contents(t.fn), "cmd1\ncmd2\ncmd2\ncmd1\n");
    
    // enqueuing a cookie that is already running
    ::unlink(t.fn);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.go();
    q.add(NULL, t.argv2[0], t.argv2);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv1[0], t.argv1);
    q.finish();
    WVPASSEQ(contents(t.fn), "cmd1\ncmd2\ncmd1\n");

    ::unlink(t.fn);
}


WVTEST_MAIN("wvsubprocqueue2")
{
    WvSubProcQueueTester t;
    WvSubProcQueue q(2);
    
    ::unlink(t.fn);
    
    // parallelism with guaranteed ordering
    ::unlink(t.fn);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv2s[0], t.argv2s);
    q.add(&t.c1, t.argv2s[0], t.argv2s);
    q.add(NULL, t.argv1[0], t.argv1);
    q.go();
    WVPASSEQ(q.running(), 2);
    q.finish();
    WVPASSEQ(contents(t.fn), "cmd1\ncmd1\ncmd2\ncmd1\n");
    
    // sequencing multiple cookies
    ::unlink(t.fn);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.go();
    WVPASSEQ(q.running(), 2);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c2, t.argv2s[0], t.argv2s);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.add(NULL, t.argv1[0], t.argv1);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.finish();
    WVPASSEQ(contents(t.fn),
	     "cmd1\ncmd1\ncmd2\ncmd1\ncmd1\ncmd2\ncmd1\ncmd1\ncmd2\n");
    
    // enqueuing cookies while running
    ::unlink(t.fn);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.go();
    WVPASSEQ(q.running(), 1);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.add(&t.c1, t.argv2[0], t.argv2);
    q.finish();
    WVPASSEQ(contents(t.fn), "cmd2\ncmd2\n");

    ::unlink(t.fn);
}
