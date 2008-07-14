#include "wvfileutils.h"
#include "wvlockfile.h"
#include "wvtest.h"

WVTEST_MAIN("lockfile blank")
{
    WvString lockname("/tmp/stupidlock.%s.tmp", getpid());
    ftouch(lockname);

    WvLockFile lockfile(lockname);
    WVPASS(lockfile.lock());
}
