/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * small program to test the wvfork() function.
 *
 * correct output is:
 *  1
 *  2
 *  3
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "wvfork.h"

int main()
{
    pid_t pid = 0;

    fcntl(1, F_SETFD, FD_CLOEXEC);	// make stdout close-on-exec

    pid = wvfork(1);		// but don't close it this time!
    if (pid != 0)
    {
	// parent
	printf("1\n");
	waitpid(pid, NULL, 0);
    }
    else
    {
	// child
	printf("2\n");

	pid = wvfork();		// close it this time.
	if (pid != 0)
	{
	    // parent
	    printf("3\n");
	    waitpid(pid, NULL, 0);
	}
	else
	{
	    // child
	    printf("4\n");	// should NOT be printed!
	}
    }
    return (0);
}
