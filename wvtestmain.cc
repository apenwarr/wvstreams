#include "wvtest.h"
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
    int startfd, endfd;
    const char *prefix = "";
    
    if (argc > 1)
	prefix = argv[1];
    
    startfd = fd_count("start");
    int ret = WvTest::run_all(prefix);
    endfd = fd_count("end");
    
    WVPASS(startfd == endfd);
    
    // keep 'make' from aborting if this environment variable is set
    if (getenv("WVTEST_NO_FAIL"))
	return 0;
    else
	return ret;
}
