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

static WvString fn("test-%s.tmp", getpid());
static WvString cmd1("echo cmd1 >>%s", fn), 
	cmd2("echo cmd2 >>%s", fn),
	cmd2s("sleep 1; %s", cmd2);
static const char *argv1[] = { "sh", "-c", cmd1, NULL };
static const char *argv2[] = { "sh", "-c", cmd2, NULL };
static const char *argv2s[] = { "sh", "-c", cmd2s, NULL };
static const char *argv3[] = { "rm", "-f", fn, NULL };
static int c1, c2; // cookies that we can point to - value doesn't matter


static WvString contents(WvStringParm fn)
{
    return WvFile(fn, O_RDONLY).getline(-1, 0);
}


static bool exists(WvStringParm fn)
{
    return !access(fn, F_OK);
}


WVTEST_MAIN("wvsubprocqueue1")
{
    ::unlink(fn);
    WVFAIL(exists(fn));

    WvSubProcQueue q(1);
    
    // basic sequencing
    ::unlink(fn);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv3[0], argv3);
    q.add(NULL, argv2[0], argv2);
    WVPASSEQ(q.remaining(), 3);
    q.go();
    WVPASSEQ(q.running(), 1);
    WVPASSEQ(q.remaining(), 3); // one running, two waiting
    q.finish();
    WVPASSEQ(q.remaining(), 0);
    WVPASS(q.isempty());
    WVPASS(exists(fn));
    WVPASSEQ(contents(fn), "cmd2\n");
    
    // cookie sequencing and duplicate detection
    ::unlink(fn);
    q.add(&c1, argv1[0], argv1);
    q.add(&c1, argv1[0], argv1);
    q.add(NULL, argv2[0], argv2);
    q.add(&c1, argv1[0], argv1);
    q.add(NULL, argv2[0], argv2);
    q.add(&c1, argv1[0], argv1);
    q.finish();
    WVPASSEQ(contents(fn), "cmd1\ncmd2\ncmd2\ncmd1\n");
    
    // enqueuing a cookie that is already running
    ::unlink(fn);
    q.add(&c1, argv1[0], argv1);
    q.go();
    q.add(NULL, argv2[0], argv2);
    q.add(&c1, argv1[0], argv1);
    q.add(&c1, argv1[0], argv1);
    q.add(&c1, argv1[0], argv1);
    q.finish();
    WVPASSEQ(contents(fn), "cmd1\ncmd2\ncmd1\n");

    ::unlink(fn);
}


WVTEST_MAIN("wvsubprocqueue2")
{
    WvSubProcQueue q(2);
    
    ::unlink(fn);
    
    // parallelism with guaranteed ordering
    ::unlink(fn);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(&c1, argv2s[0], argv2s);
    q.add(&c1, argv2s[0], argv2s);
    q.add(NULL, argv1[0], argv1);
    q.go();
    WVPASSEQ(q.running(), 2);
    q.finish();
    WVPASSEQ(contents(fn), "cmd1\ncmd1\ncmd2\ncmd1\n");
    
    // sequencing multiple cookies
    ::unlink(fn);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(&c1, argv2[0], argv2);
    q.go();
    WVPASSEQ(q.running(), 2);
    q.add(NULL, argv1[0], argv1);
    q.add(&c1, argv2[0], argv2);
    q.add(NULL, argv1[0], argv1);
    q.add(&c2, argv2s[0], argv2s);
    q.add(NULL, argv1[0], argv1);
    q.add(&c1, argv2[0], argv2);
    q.add(NULL, argv1[0], argv1);
    q.add(&c1, argv2[0], argv2);
    q.finish();
    WVPASSEQ(contents(fn),
	     "cmd1\ncmd1\ncmd2\ncmd1\ncmd1\ncmd2\ncmd1\ncmd1\ncmd2\n");
    
    // enqueuing cookies while running
    ::unlink(fn);
    q.add(&c1, argv2[0], argv2);
    q.go();
    WVPASSEQ(q.running(), 1);
    q.add(&c1, argv2[0], argv2);
    q.add(&c1, argv2[0], argv2);
    q.finish();
    WVPASSEQ(contents(fn), "cmd2\ncmd2\n");

    ::unlink(fn);
}
