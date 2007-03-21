#include "wvsubprocqueuestream.h"
#include "wvistreamlist.h"
#include "wvtest.h"

WVTEST_MAIN("wvsubprocqueuestream")
{
    WvIStreamList l;
    WvSubProcQueueStream q(2);
    l.append(&q, false, "subproc queue");
    
    const char *argv1[] = { "true", NULL };
    const char *argv2[] = { "sleep", "1", NULL };
    
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv2[0], argv2);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv2[0], argv2);
    q.add(NULL, argv1[0], argv1);
    q.add(NULL, argv1[0], argv1);
    
    // this test just makes sure all processes do finish within a reasonable
    // timeout.  There are 11 processes to run, and the stream uses alarm(100)
    // unless it actually starts a new process, and these processes are pretty
    // short-lived except for the sleeps, so it should really take no
    // more than 25 passes or so.  But we'll give it more, just in case the
    // system is loaded down.  (Hint: if the alarm() is too short or infinite,
    // we'll have too many passes or the test will freeze.)
    int i;
    for (i = 0; i < 55 && !q.isempty(); i++)
    {
	printf("#%d %ld: running with %d total, %d waiting.\n",
	       i, (long)time(NULL), q.remaining(), q.running());
	l.runonce(-1);
    }
    printf("Done looping with i=%d.\n", i);
    WVPASS(i < 50);
    WVPASSEQ(q.remaining(), 0);
}
