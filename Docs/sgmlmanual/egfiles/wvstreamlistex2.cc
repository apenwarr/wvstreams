/** \file 
 * A WvStream example.
 */
/** \example wvstreamlistex2.cc
 * Some text about this example...
 */

#include <wvstreamlist.h>
#include <wvpipe.h>

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
    
    WvStreamList l;
    l.append(&stream1, false);
    l.append(&stream2, false);
    
    while (stream1.isok() || stream2.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
}
