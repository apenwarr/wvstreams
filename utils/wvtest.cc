/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Part of an automated testing framework.  See wvtest.h.
 */
#include "wvtest.h"
#include "wvautoconf.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
# include <valgrind/valgrind.h>
#else
# warning "fake valgrind"
# define VALGRIND_COUNT_ERRORS 0
# define VALGRIND_DO_LEAK_CHECK
# define VALGRIND_COUNT_LEAKS(a,b,c,d) (a=b=c=d=0)
#endif

static int memerrs()
{
    return (int)VALGRIND_COUNT_ERRORS;
}

static int memleaks()
{
    int leaked = 0, dubious = 0, reachable = 0, suppressed = 0;
    VALGRIND_DO_LEAK_CHECK;
    VALGRIND_COUNT_LEAKS(leaked, dubious, reachable, suppressed);
    printf("memleaks: sure:%d dubious:%d reachable:%d suppress:%d\n",
	   leaked, dubious, reachable, suppressed);
    
    // dubious+reachable are normally non-zero because of globals...
    // return leaked+dubious+reachable;
    return leaked;
}


WvTest *WvTest::first, *WvTest::last;
int WvTest::fails, WvTest::runs;


WvTest::WvTest(const char *_descr, const char *_idstr, MainFunc *_main)
{
    const char *cptr = strrchr(_idstr, '/');
    if (cptr)
	idstr = cptr+1;
    else
	idstr = _idstr;
    descr = _descr;
    main = _main;
    next = NULL;
    if (first)
	last->next = this;
    else
	first = this;
    last = this;
}


int WvTest::run_all(const char *prefix)
{
    int old_valgrind_errs = 0, new_valgrind_errs;
    int old_valgrind_leaks = 0, new_valgrind_leaks;
    
    fails = runs = 0;
    for (WvTest *cur = first; cur; cur = cur->next)
    {
	if (!prefix
	    || !strncasecmp(cur->idstr, prefix, strlen(prefix))
	    || !strncasecmp(cur->descr, prefix, strlen(prefix)))
	{
	    printf("Testing \"%s\" in %s:\n", cur->descr, cur->idstr);
	    cur->main();
	    
	    new_valgrind_errs = memerrs();
	    WVPASS(new_valgrind_errs == old_valgrind_errs);
	    old_valgrind_errs = new_valgrind_errs;
	    
	    new_valgrind_leaks = memleaks();
	    WVPASS(new_valgrind_leaks == old_valgrind_leaks);
	    old_valgrind_leaks = new_valgrind_leaks;
	    
	    printf("\n");
	}
    }
    
    if (prefix && prefix[0])
	printf("WvTest: only ran tests starting with '%s'.\n", prefix);
    else
	printf("WvTest: ran all tests.\n");
    printf("WvTest: %d test%s, %d failure%s.\n",
	   runs, runs==1 ? "" : "s",
	   fails, fails==1 ? "": "s");
    
    return fails != 0;
}


void WvTest::start(const char *file, int line, const char *condstr)
{
    const char *cptr = strrchr(file, '/');
    if (!cptr)
	cptr = file;
    else
	cptr++;
    printf("! %s:%-5d %-40s ", cptr, line, condstr);
    fflush(stdout);
}


void WvTest::check(bool cond)
{
    runs++;
    
    if (cond)
	printf("ok\n");
    else
    {
	printf("FAILED\n");
	fails++;
    }
    fflush(stdout);
}


