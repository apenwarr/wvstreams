#include <time.h>

#include "wvdailyevent.h"
#include "wvconfemu.h"
#include "uniinigen.h"

int main(int argc, char *argv[])
{
    WvDailyEvent x(1, 1);

    //UniConfRoot cfg("ini:/tmp/foo.ini");

    time_t now = time(NULL);
    wvcon->print("Now is %s", ctime(&now));

    x.configure(0, 60*24*15, false);

    time_t next = x.next_event();
    wvcon->print("Next event %s seconds, %s", next-now, ctime(&next));

    while (1)
    {
	if (x.select(100))
	{
	    now = time(NULL);
	    wvcon->print("Triggering at %s", ctime(&now));
	    x.callback();
	    next = x.next_event();
    	    wvcon->print("Next event %s seconds, %s", next-now, ctime(&next));
	}
    }
    
    return 0;
}

