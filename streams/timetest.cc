/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
 */
#include "wvtimestream.h"
#include "wvlog.h"
#include <sys/time.h>
#include <unistd.h>

int main()
{
    WvLog log("timetest", WvLog::Info);
    WvTimeStream t;
    int count;
    
    free(malloc(1));
    
    log("Artificial burstiness test - should take exactly 10 seconds\n");
    
    t.set_timer(100);

    for (count = 0; count < 100; count++)
    {
	if (!(count % 10)) log("\n");
	
	while (!t.select(5*(100-count))) 
	    ;
	t.callback();
	
	log("%02s ", count);
    }
    
    return 0;
}
