/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvTimeoutStream test.  Should only fire once.
 */

#include "wvtimeoutstream.h"
#include "wvlog.h"
#include <sys/time.h>

WvLog mylog("timeouttest", WvLog::Info);

void timeout(WvStream &s, void *userdata)
{
    static int count = 0;
    count++;
    mylog("Fire %s\n", count);
}

int main()
{
    WvTimeoutStream t(1000);
    t.setcallback(timeout, NULL);

    free(malloc(1));
  
    for (int i = 0; i < 3 && t.isok(); i++)
    {
        if (t.select(-1))
            t.callback();
    }
    
    return 0;
}
