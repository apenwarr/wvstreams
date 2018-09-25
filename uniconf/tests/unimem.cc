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
    printf("uniconfvaluetree: %d bytes\n", (int)sizeof(UniConfValueTree));
    printf("wvstring: %d bytes\n", (int)sizeof(WvString));
    Report r;

    int mode = 2;
    switch (mode)
    {
    case -1:
	{
	    UniConfRoot uni;
	    r.go();
	    uni.mount("ini:/tmp/dns.ini2", true);
	    r.go();
	    system("touch /tmp/dns.ini2");
	    uni.refresh();
	    r.go();
	    system("touch /tmp/dns.ini2");
	    uni.refresh();
	    r.go();
	    for (int x = 0; x < 1e8; x++)
		;
	    system("touch /tmp/dns.ini2");
	    uni.refresh();
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
    case 2:
	{
	    UniConfRoot uni("unix:/tmp/foos");
	    r.go();
	    {
		UniConf::RecursiveIter i(uni);
		r.go();
	    }
	    r.go();
	}
    }

    r.go();
    return 0;
}
