/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Part of an automated testing framework.  See wvtest.h.
 */
#include "wvtest.h"
#include "wvautoconf.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <signal.h>

#include <cstdlib>

#ifdef HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
# include <valgrind/valgrind.h>
#else
# define VALGRIND_COUNT_ERRORS 0
# define VALGRIND_DO_LEAK_CHECK
# define VALGRIND_COUNT_LEAKS(a,b,c,d) (a=b=c=d=0)
#endif

#define MAX_TEST_TIME 40     // max seconds for a single test to run
#define MAX_TOTAL_TIME 120*60 // max seconds for the entire suite to run

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
    fflush(stdout);
    
    // dubious+reachable are normally non-zero because of globals...
    // return leaked+dubious+reachable;
    return leaked;
}


WvTest *WvTest::first, *WvTest::last;
int WvTest::fails, WvTest::runs;
time_t WvTest::start_time;


void WvTest::alarm_handler(int)
{
    printf("\n! WvTest  Current test took longer than %d seconds!  FAILED\n",
	   MAX_TEST_TIME);
    fflush(stdout);
    abort();
}


static const char *pathstrip(const char *filename)
{
    const char *cptr;
    cptr = strrchr(filename, '/');
    if (cptr) filename = cptr + 1;
    cptr = strrchr(filename, '\\');
    if (cptr) filename = cptr + 1;
    return filename;
}


WvTest::WvTest(const char *_descr, const char *_idstr, MainFunc *_main,
	       int _slowness)
{
    idstr = pathstrip(_idstr);
    descr = _descr;
    main = _main;
    slowness = _slowness;
    next = NULL;
    if (first)
	last->next = this;
    else
	first = this;
    last = this;
}


static bool prefix_match(const char *s, const char * const *prefixes)
{
    for (const char * const *prefix = prefixes; prefix && *prefix; prefix++)
    {
	if (!strncasecmp(s, *prefix, strlen(*prefix)))
	    return true;
    }
    return false;
}


int WvTest::run_all(const char * const *prefixes)
{
    int old_valgrind_errs = 0, new_valgrind_errs;
    int old_valgrind_leaks = 0, new_valgrind_leaks;
    
#ifdef _WIN32
    /* I should be doing something to do with SetTimer here, 
     * not sure exactly what just yet */
#else
    signal(SIGALRM, alarm_handler);
    // signal(SIGALRM, SIG_IGN);
    alarm(MAX_TEST_TIME);
#endif
    start_time = time(NULL);
    
    // make sure we can always start out in the same directory, so tests have
    // access to their files.  If a test uses chdir(), we want to be able to
    // reverse it.
    char wd[1024];
    if (!getcwd(wd, sizeof(wd)))
	strcpy(wd, ".");
    
    const char *slowstr1 = getenv("WVTEST_MIN_SLOWNESS");
    const char *slowstr2 = getenv("WVTEST_MAX_SLOWNESS");
    int min_slowness = 0, max_slowness = 65535;
    if (slowstr1) min_slowness = atoi(slowstr1);
    if (slowstr2) max_slowness = atoi(slowstr2);
    
    // there are lots of fflush() calls in here because stupid win32 doesn't
    // flush very often by itself.
    fails = runs = 0;
    for (WvTest *cur = first; cur; cur = cur->next)
    {
	if (cur->slowness <= max_slowness
	    && cur->slowness >= min_slowness
	    && (!prefixes
		|| prefix_match(cur->idstr, prefixes)
		|| prefix_match(cur->descr, prefixes)))
	{
	    printf("Testing \"%s\" in %s:\n", cur->descr, cur->idstr);
	    fflush(stdout);
	    
	    cur->main();
	    chdir(wd);
	    
	    new_valgrind_errs = memerrs();
	    WVPASS(new_valgrind_errs == old_valgrind_errs);
	    old_valgrind_errs = new_valgrind_errs;
	    
	    new_valgrind_leaks = memleaks();
	    WVPASS(new_valgrind_leaks == old_valgrind_leaks);
	    old_valgrind_leaks = new_valgrind_leaks;
	    
	    fflush(stderr);
	    printf("\n");
	    fflush(stdout);
	}
    }
    
    if (prefixes && *prefixes)
	printf("WvTest: WARNING: only ran tests starting with "
	       "specifed prefix(es).\n");
    else
	printf("WvTest: ran all tests.\n");
    printf("WvTest: %d test%s, %d failure%s.\n",
	   runs, runs==1 ? "" : "s",
	   fails, fails==1 ? "": "s");
    fflush(stdout);
    
    return fails != 0;
}


void WvTest::start(const char *file, int line, const char *condstr)
{
    const char *file2 = pathstrip(file);
    
    char *condstr2 = strdup(condstr);
    for (char *cptr = condstr2; *cptr; cptr++)
    {
	if (!isprint((unsigned char)*cptr))
	    *cptr = '!';
    }
    
    printf("! %s:%-5d %-40s ", file2, line, condstr2);
    fflush(stdout);

    free(condstr2);
}


void WvTest::check(bool cond)
{
#ifndef _WIN32
    alarm(MAX_TEST_TIME); // restart per-test timeout
#endif
    if (!start_time) start_time = time(NULL);
    
    if (time(NULL) - start_time > MAX_TOTAL_TIME)
    {
	printf("\n! WvTest   Total run time exceeded %d seconds!  FAILED\n",
	       MAX_TOTAL_TIME);
	fflush(stdout);
	abort();
    }
    
    runs++;
    
    if (cond)
    {
	printf("ok\n");
	fflush(stdout);
    }
    else
    {
	printf("FAILED\n");
	fflush(stdout);
	fails++;
	
	if (getenv("WVTEST_DIE_FAST"))
	    abort();
    }
}


bool WvTest::start_check_eq(const char *file, int line,
			    const char *a, const char *b, bool expect_pass)
{
    if (!a) a = "";
    if (!b) b = "";
    
    size_t len = strlen(a) + strlen(b) + 8 + 1;
    char *str = new char[len];
    sprintf(str, "[%s] %s [%s]", a, expect_pass ? "==" : "!=", b);
    
    start(file, line, str);
    delete[] str;
    
    bool cond = !strcmp(a, b);
    if (!expect_pass)
        cond = !cond;

    check(cond);
    return cond;
}


bool WvTest::start_check_eq(const char *file, int line, 
                            int a, int b, bool expect_pass)
{
    size_t len = 128 + 128 + 8 + 1;
    char *str = new char[len];
    sprintf(str, "%d %s %d", a, expect_pass ? "==" : "!=", b);
    
    start(file, line, str);
    delete[] str;
    
    bool cond = (a == b);
    if (!expect_pass)
        cond = !cond;

    check(cond);
    return cond;
}

