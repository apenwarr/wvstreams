#include "wvtest.h"
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <fcntl.h>
#include <errno.h>

/*
 * The main point of these tests is for win32, because we override
 * the close/read/write functions there.  read/write get tested easily
 * elsewhere, but close() is harder to check.
 * 
 * So the rule is: if reopening a new socket gives the same fd as last
 * time, you know the old one must be gone.
 */

static void fd0_ok()
{
    int fd = dup(0);
    if (!WVPASS(fd > 0))
	perror("dup(0)");
    close(fd);
}

static void fdcycle(int base)
{
    int fd1 = dup(base);
    WVPASS(fd1 >= 0);
    int fd2 = dup(base);
    WVPASS(fd2 >= 0);
    int fd3 = dup(base);
    WVPASS(fd3 >= 0);
    
    close(fd1);
    WVPASSEQ(dup(base), fd1);
    close(fd2);
    close(fd3);
    WVPASSEQ(dup(fd1), fd2);
    WVPASSEQ(dup(fd2), fd3);
    
    close(fd1);
    close(fd2);
    close(fd3);
    fd0_ok();

#if WINE_DIDNT_FAIL_THIS_TEST
    close(fd3);
    WVPASSEQ(errno, EBADF);
#endif
    fd0_ok();
}


WVTEST_MAIN("fd-plain recycling tests")
{
    fdcycle(0);
    fd0_ok();
}


WVTEST_MAIN("fd-socket recycling tests")
{
    errno = 0;
    int sfd1 = socket(PF_INET, SOCK_STREAM, 0);
    WVPASS(sfd1 >= 0) || fprintf(stderr, "error was: %d\n", errno);
    int sfd2 = socket(PF_INET, SOCK_STREAM, 0);
    WVPASS(sfd2 >= 0) || fprintf(stderr, "error was: %d\n", errno);
    close(sfd1);
#ifndef WIN32
    WVPASSEQ(socket(PF_INET, SOCK_STREAM, 0), sfd1); //FIXME: Fails in VC++
#endif 
    close(sfd2);
#ifndef WIN32
    WVPASSEQ(socket(PF_INET, SOCK_STREAM, 0), sfd2);//FIXME: Fails in VC++
#endif 
    fdcycle(0);
    close(sfd1);
    close(sfd2);
    
#if WINE_DIDNT_FAIL_THIS_TEST
    close(sfd2);
    WVPASSEQ(errno, EBADF);
#endif
}
