#include "wvgdbmlist.h"

int main()
{
    WvGdbmList<WvString> list("dbmlist");
    list.zap();
    printf("empty:%d\n", list.isempty());
    printf("count:%d\n", list.count());
    list.append("honky 1");
    list.append("honky 2");
    list.append("honky 3");
    list.append("honky 4");
    list.append("honky 5");
    list.append("honky 6");
    list.append("honky 77");
    list.prepend("honky 0");
    printf("empty:%d\n", list.isempty());
    printf("count:%d\n", list.count());
    printf("'%s'\n", list.first()->cstr());
    
    bool odd = false;
    printf("\nIterator test:\n");
    WvGdbmList<WvString>::Iter i(list);
    for (i.rewind(); i.next(); )
    {
	printf("    %5d = ", i.cur());
	fflush(stdout);
	printf("'%s'\n", i->cstr());
	if (odd)
	    i.xunlink();
	odd = !odd;
    }
    
    list.unlink_first();
    
    printf("\nIterator test:\n");
    for (i.rewind(); i.next(); )
    {
	printf("    %5d = ", i.cur());
	fflush(stdout);
	printf("'%s'\n", i->cstr());
    }
    
    for (int xcount = 0; xcount < 1000000; xcount++)
    {
	list.append(new WvString("yak %s", xcount), true);
	list.prepend(new WvString("bakko %s", xcount), true);
	
	int count = 0;
	for (i.rewind(); i.next(); count++)
	{
	    printf("'%s'\n", i->cstr());
//	    if (count < 2)
//		i.xunlink();
	}
    }
    
    return 0;
}

