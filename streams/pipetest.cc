/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvpipe.h"
#include "wvlog.h"

int main(int argc, char **argv)
{
    const char *av[] = {
	argc==2 ? argv[1] : "/bin/bash",
	NULL
    };
    
    WvLog log(av[0]);
    WvPipe p(av[0], av, true, true, true);
    
    wvcon->autoforward(p);
    p.autoforward(log);
    
    p.write("test string\r\n");
    
    while (p.isok() && wvcon->isok())
    {
	if (p.select(100))
	    p.callback();
	if (wvcon->select(100))
	    wvcon->callback();
    }
    
    if (p.child_exited())
	log(WvLog::Notice, "Exited (return code == %s)\n",
	    p.exit_status());
    
    return 0;
}
