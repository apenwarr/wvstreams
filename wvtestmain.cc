#include "wvtest.h"
#include "wvstring.h"
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>

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
    // test wvtest itself.  Not very thorough, but you have to draw the
    // line somewhere :)
    WVPASS(true);
    WVPASS(1);
    WVFAIL(false);
    WVFAIL(0);
    int startfd, endfd;
    char * const *prefixes = NULL;
    
    if (argc > 1)
	prefixes = argv + 1;
    
    startfd = fd_count("start");
    int ret = WvTest::run_all(prefixes);
    endfd = fd_count("end");
    
    WVPASS(startfd == endfd);
    //if (startfd != endfd)
    {
	system(WvString("ls -l /proc/%s/fd", getpid()));
    }
    
    // keep 'make' from aborting if this environment variable is set
    if (getenv("WVTEST_NO_FAIL"))
	return 0;
    else
	return ret;
}
