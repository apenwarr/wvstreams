#include "wvtest.h"
#include "wvpty.h"
#include "wvpipe.h"

WVTEST_MAIN("stty")
{
    const char *argv[] = { "stty", NULL };

    WvPty pty(argv[0], argv);
    pty.autoforward(*wvout);
    while (!pty.child_exited())
    {
	printf("runonce\n");
        pty.runonce();
    }
    WVPASS("process exited");

    // pty is a tty!  exit code should show success.
    WVPASSEQ(pty.finish(), 0); 

    WvPipe pipe(argv[0], argv, true, true, true);
    pipe.autoforward(*wvout);
    while (!pipe.child_exited())
    {
	printf("runonce\n");
        pipe.runonce();
    }
    // pipe is not a tty.  exit code should show failure.
    WVFAILEQ(pipe.finish(), 0);
}
