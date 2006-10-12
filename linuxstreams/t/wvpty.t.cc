#include "wvpty.h"
#include "wvpipe.h"
#include "wvistreamlist.h"
#include "wvtest.h"

WVTEST_MAIN("stty")
{
    const char *argv[] = { "stty", NULL };

    {
	WvIStreamList l;
	WvPty pty(argv[0], argv);
	pty.autoforward(*wvout);
	l.append(&pty, false);
	while (!pty.child_exited() || pty.isok())
	{
	    printf("runonce\n");
	    l.runonce(100);
	}
	WVPASS("process exited");

	// pty is a tty!  exit code should show success.
	WVPASSEQ(pty.finish(), 0); 
    }

    {
	WvIStreamList l;
	WvPipe pipe(argv[0], argv, true, true, true);
	pipe.autoforward(*wvout);
	l.append(&pipe, false);
	while (!pipe.child_exited() || pipe.isok())
	{
	    printf("runonce\n");
	    l.runonce(100);
	}
	// pipe is not a tty.  exit code should show failure.
	WVFAILEQ(pipe.finish(), 0);
    }
}
