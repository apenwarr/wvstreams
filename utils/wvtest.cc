/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Part of an automated testing framework.  See wvtest.h.
 */
#include "wvtest.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>


WvTest *WvTest::first, *WvTest::last;
int WvTest::fails, WvTest::runs;


WvTest::WvTest(const char *_idstr, MainFunc *_main)
{
    const char *cptr = strrchr(_idstr, '/');
    if (cptr)
	idstr = cptr+1;
    else
	idstr = _idstr;
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
    fails = runs = 0;
    for (WvTest *cur = first; cur; cur = cur->next)
    {
	if (!prefix || !strncasecmp(cur->idstr, prefix, strlen(prefix)))
	{
	    cur->main();
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


