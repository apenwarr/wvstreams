#include "uniconfroot.h"
#include <unistd.h>

class Report
{
public:
    const char *before;
    
    void ps()
    {
	system(WvString("ps -o pid,sz,vsz,rss,trs,drs,dsiz,cmd %s",
			getpid()));
    }
    
    Report()
    {
	before = (const char *)sbrk(0);
	ps();
    }
    
    void go()
    {
	const char *after = (const char *)sbrk(0);
	ps();
	printf("%p, %p, %ld\n", before, after, (long)(after-before));
    }
};

int main()
{
    printf("uniconfvaluetree: %d bytes\n", sizeof(UniConfValueTree));
    printf("wvstring: %d bytes\n", sizeof(WvString));
    Report r;
    
    int mode = -1;
    switch (mode)
    {
    case -1:
	{
	    UniConfRoot uni("ini:/tmp/dns.ini");
	    r.go();
	}
	break;
    case 0:
	{
	    UniConfRoot uni("temp:");
	    WvString s("this is a big long line with a really big "
		       "long string involved in it somehow");
	    for (int i = 0; i < 18000; i++)
		uni.xset(WvString("blah/pah/%s", i), s.edit());
	    uni.commit();
	    r.go();
	    uni.remove();
	    uni.commit();
	    r.go();
	}
	break;
    case 1:
	{
	    WvStringList l;
	    WvString s("this is a big long line with a really big "
		       "long string involved in it somehow");
	    WvString a[18000];
	    for (int i = 0; i < 18000; i++)
		l.append(&(a[i] = s), false);
	    r.go();
	}
	break;
    }
    
    return 0;
}
