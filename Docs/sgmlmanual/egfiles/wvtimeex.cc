/*
 * A WvTimeStream example.
 *
 * This program should take exactly ten seconds to run, but
 * tests how well the time stream handles being executed in bursts.
 */

#include "wvtimestream.h"
#include "wvlog.h"
#include <sys/time.h>

int main()
{
    WvLog log("time", WvLog::Info);
    WvTimeStream t;
    int count;
    
    log("Artificial burstiness - should take exactly 10 seconds\n");

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
