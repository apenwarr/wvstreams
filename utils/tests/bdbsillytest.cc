/*
 * Do some repetitive stuff that *shouldn't* stress bdbhash very much, so
 * that somebody can strace it and see if it does.
 */
#include "wvautoconf.h"

#if defined(WITH_BDB)
#include "wvondiskhash.h"
#include "wvstring.h"

typedef WvOnDiskHash<int, WvString, WvBdbHash> MyHash;

int main()
{
    MyHash h("/tmp/blahdb");
    
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
    int num = 0;
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
#else
int main(int argc, char *argv[])
{
  return 0;
}
#endif
