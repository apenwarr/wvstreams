/*
 * A WvTimeOut example.
 *
 * Should only fire once.
 */

#include "wvtimeoutstream.h"
#include "wvlog.h"
#include <sys/time.h>

WvLog log("timeout", WvLog::Info);

void timeout(WvStream &s, void *userdata)
{
    static int count = 0;
    count++;
    log("Fire %s\n", count);
}

int main()
{
    WvTimeoutStream t(1000);
    t.setcallback(timeout, NULL);

    for (int i = 0; i < 3 && t.isok(); i++)
    {
        if (t.select(-1))
            t.callback();
    }

    return 0;
}
