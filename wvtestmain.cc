#include "wvtest.h"
#include "wvstring.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

static int fd_count(const char *when)
{
    long arg = 0;
    int count = 0;
    
    printf("fds open at %s:", when);
    
    for (int fd = 0; fd < 1024; fd++)
    {
	if (!fcntl(fd, F_GETFD, &arg))
	{
	    count++;
	    printf(" %d", fd);
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
