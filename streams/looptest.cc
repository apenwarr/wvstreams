/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 *
 * WvLoopback test.  Forks and has each process to write to the other.
 */
#include "wvloopback.h"
#include "wvlog.h"

int main()
{
    WvLoopback loop;
    WvLog log("loopy", WvLog::Info);
    char buf[1024];
    size_t size;
    pid_t pid;
    
    pid = fork();
    
    // each line should print twice, but no one guarantees in what order!
    loop.print("blah\n");
    loop.print("weasels!\n");
    
    if (pid) // parent only
    {
	while (loop.select(100))
	{
	    size = loop.read(buf, sizeof(buf));
	    log.write(buf, size);
	}
    }
    else
	_exit(0); // do not run destructors in the child
    
    return 0;
}
