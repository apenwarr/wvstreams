/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * WvPipe test.  Allows you to enter bash commands, runs them, and pipes the
 * output back to you.
 */

#include "wvpipe.h"
#include "wvlog.h"

int main(int argc, char **argv)
{
    const char *_av[] = {
	"/bin/bash",
	NULL
    };
    const char **av = (argc < 2) ? _av : (const char **)(argv + 1);
    
    WvLog log(av[0]);
    WvPipe p(av[0], av, true, false, false);
    
    wvcon->autoforward(p);
    p.autoforward(*wvcon);
    
    p.write("test string\r\n");
    
    while (p.isok() && wvcon->isok())
    {
	if (p.select(100))
	    p.callback();
	if (wvcon->select(100))
	    wvcon->callback();
    }
    
    p.flush_then_close(50000);
    while (p.isok())
    {
	log("Flushing...\n");
	if (p.select(1000))
	    p.callback();
    }
    
    if (p.child_exited())
	log(WvLog::Notice, "Exited (return code == %s)\n",
	    p.exit_status());
    
    _exit(0); // don't kill the subtask
    
    return 0;
}
