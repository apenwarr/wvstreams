/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvconf.h"

WVTEST_MAIN("cfgsection NULL name get test")
{
    WvConf cfg("/tmp/foo.ini");
    cfg.set("Main", "Test", "Test");

    // If the next test fails, we will segfault the tester.
    // Make sure our messages to this point get printed if
    // we _do_ segfault.
    fflush(stdout);

    WVPASS(!cfg.getint("Main", WvString::null, 0));
}
