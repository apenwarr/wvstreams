#include "wvargs.h"
#include "wvtest.h"
#include "wvcrash.h"
#include "wvstring.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

static bool fd_is_valid(int fd)
{
#ifdef _WIN32
    if ((HANDLE)_get_osfhandle(fd) != INVALID_HANDLE_VALUE) return true;
#endif    
    int nfd = dup(fd);
    if (nfd >= 0)
    {
	close(nfd);
	return true;
    }
    return false;

}


static int fd_count(const char *when)
{
    int count = 0;
    
    printf("fds open at %s:", when);
    
    for (int fd = 0; fd < 1024; fd++)
    {
	if (fd_is_valid(fd))
	{
	    count++;
	    printf(" %d", fd);
	    fflush(stdout);
	}
    }
    printf("\n");
    
    return count;
}


int main(int argc, char **argv)
{
    WvArgs args;
    WvStringList prefixes;
    
#ifdef _WIN32
    bool ccrash = true;
    args.add_reset_bool_option('w', "wcrash", "enable windows crash", ccrash);
#endif

    if (!args.process(argc, argv, &prefixes))
    {
	args.print_usage(argc, argv);
        return 1;
    }

#ifdef _WIN32
    if (ccrash)
	setup_console_crash();
#endif

    // test wvtest itself.  Not very thorough, but you have to draw the
    // line somewhere :)
    WVPASS(true);
    WVPASS(1);
    WVFAIL(false);
    WVFAIL(0);
    int startfd, endfd;
        
    startfd = fd_count("start");
    int ret = WvTest::run_all(prefixes);
    endfd = fd_count("end");
    
    WVPASS(startfd == endfd);
#ifndef _WIN32
    if (startfd != endfd)
	system(WvString("ls -l /proc/%s/fd", getpid()));
#endif    
    
    // keep 'make' from aborting if this environment variable is set
    if (getenv("WVTEST_NO_FAIL"))
	return 0;
    else
	return ret;
}
