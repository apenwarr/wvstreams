#include "wvsystem.h"
#include "wvfile.h"
#include "wvtest.h"


WVTEST_MAIN("wvsystem")
{
    WvString fn("test-%s.tmp", getpid()); 
    WvString fn2("test2-%s.tmp", getpid()); 
    WvString fn3("test3-%s.tmp", getpid());
    WvString teststring("test \t string");
    
    ::unlink(fn);
    WVPASS(access(fn, F_OK) != 0);
    WVPASSEQ(WvSystem("ls", "-l").outfile(fn).go(), 0);
    WVPASS(!access(fn, F_OK));
    
    const char *argv1[] = { "ls", "-l", fn, NULL };
    const char *argv2[] = { "ls", "-l", "random-nonexistent-file", NULL };
    WVPASSEQ(WvSystem(argv1).go(), 0);
    WVPASS(WvSystem(argv2).go() != 0);
    
    // whitespace not preserved due to inadequate quoting
    system(WvString("echo %s >%s", teststring, fn));
    WVPASSEQ(WvFile(fn, O_RDONLY).getline(-1), "test string");
    
    // whitespace preserved because quoting not needed
    WvSystem("echo", teststring).outfile(fn);
    WVPASSEQ(WvFile(fn, O_RDONLY).getline(-1), teststring);
    
    // WvSystem doesn't run the command until it gets destroyed
    ::unlink(fn2);
    {
	WvSystem x("echo", teststring);
	x.outfile(fn2);
	WVFAIL(!access(fn2, F_OK));
    }
    WVPASS(!access(fn2, F_OK));
    
    // make sure infile, outfile, and errfile all work properly
    ::unlink(fn);
    ::unlink(fn2);
    ::unlink(fn3);
    WvSystem("echo", teststring, "x\n", teststring).outfile(fn);
    WvSystem("cat", "-", "stupid").infile(fn).outfile(fn2).errfile(fn3);
    WvString outstr("%s x\n %s\n", teststring, teststring);
    WVPASSEQ(WvFile(fn, O_RDONLY).getline(-1, 0), outstr);
    WVPASSEQ(WvFile(fn2, O_RDONLY).getline(-1, 0), outstr);
    WVPASS(!!WvString(WvFile(fn3, O_RDONLY).getline(-1)));
    
    ::unlink(fn);
    ::unlink(fn2);
    ::unlink(fn3);
}
