
#include "wvmagiccircle.h"
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


int main()
{
    WvMagicCircle q(999);
    char buf[1024], buf2[1024];
    size_t len;
    size_t i;
    
    srandom(0);
    
    if (q.geterr())
    {
	fprintf(stderr, "q: %s\n", q.errstr().cstr());
	return 1;
    }
    
    if (fork() > 0)
    {
	// parent
	for (i = 1; i < 1000; i++)
	{
	    while (q.used() < i)
	    {
		//usleep(1*1000);
		//printf(".");
		//fflush(stdout);
	    }
	    
	    q.skip(1);
	    len = q.get(buf, i-1);
	    
	    memset(buf2, i % 256, i);
	    
	    if (i == 999)
	    {
		assert(memcmp(buf, buf2, i));
		printf("Comparison failed at 999, as expected.  Done!\n");
		break;
	    }
	    
	    assert(!memcmp(buf, buf2, i-1));
	    
	    printf("         << %d\n", i);
	}
	
	assert(!q.used());
    }
    else 
    {
	// child
	for (i = 1; i <= 999; i++)
	{
	    printf(">> %d\n", i);
	    
	    memset(buf, i % 256, i);
	    
	    if (i == 999)
		buf[5]++;
	    
	    while (q.left() < i)
		; //usleep(1*1000);
	    
	    q.put(buf, i);
	}
    }
    
    return 0;
}
