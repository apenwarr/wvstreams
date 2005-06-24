#include "wvcrash.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    wvcrash_setup(argv[0], "BLAHBLAH");
    
    int fd, count = 0;
    while ((fd = socket(PF_INET, SOCK_STREAM, 0)) >= 0)
    {
	fcntl(fd, F_SETFD, 0); // *not* close-on-exec
	count++;
    }
    printf("Got %d sockets.\n", count);
    
    // all fds are now in use; let's see if wvcrash can handle it!
    
    abort();
    
    return 0;
}
