#include "wvtest.h"
#include "wvpty.h"
#include "wvpipe.h"

WVTEST_MAIN("stty")
{
    const char *argv[] = { "stty", NULL };

    WvPty pty(argv[0], argv);
    pty.autoforward(*wvout);
    while (!pty.child_exited())
        pty.runonce();
    // pty is a tty!
    WVPASS(pty.finish() == 0); 

    WvPipe pipe(argv[0], argv, true, true, true);
    pipe.autoforward(*wvout);
    while (!pipe.child_exited())
        pipe.runonce();
    // pipe is not a tty...
    WVPASS(pipe.finish() != 0);
}
