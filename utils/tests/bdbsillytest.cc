/*
 * Do some repetitive stuff that *shouldn't* stress bdbhash very much, so
 * that somebody can strace it and see if it does.
 */
#include "wvbdbhash.h"

typedef WvBdbHash<int, WvString> MyHash;

int main()
{
    MyHash h("/tmp/blahdb");
    int num;
    
    fprintf(stderr, "Zapping...");
    h.zap();
    fprintf(stderr, "done\n");
    
    fprintf(stderr, "Adding...");
    for (int count = 0; count < 1000000; count++)
    {
	h.add(count, WvString("String number %s", count), false);
	//fprintf(stderr, ".");
    }
    fprintf(stderr, "done\n");
#if 0    
    num = 0;
    MyHash::Iter i(h);
    for (i.rewind(), num = 0; i.next(); num++)
    {
	for (int count = 0; count < 10; count++)
	{
	    int cur = i.key();
	    assert(num == cur);
	    fprintf(stderr, ":");
	    WvString scur(*i);
	    WvString snum("String number %s", num);
	    assert(snum == scur);
	    fprintf(stderr, ".");
	}
	
	fprintf(stderr, "!");
    }
#endif
}
