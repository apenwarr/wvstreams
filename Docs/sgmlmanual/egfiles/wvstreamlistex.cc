/** \file
 * A WvPipe example.
 */
/** \example wvstringlistex.cc
 * Some text about this example...
 */

#include <wvpipe.h>

int X = -1;
// int X = 0;
// int X = 1000;

int main()
{
    const char *argv1[] = { "sh", "-c", 
			    "while :; do echo foo; sleep 3; done", NULL };
    const char *argv2[] = { "sh", "-c", 
			    "while :; do echo snorkle; sleep 1; done", NULL };

    WvPipe stream1(argv1[0], argv1, false, true, false);
    WvPipe stream2(argv2[0], argv2, false, true, false);
    stream1.autoforward(*wvcon);
    stream2.autoforward(*wvcon);
    
    while (stream1.isok() || stream2.isok())
    {
	if (stream1.select(X))
	    stream1.callback();
	if (stream2.select(X))
	    stream2.callback();
    }
}
