#include "wvtest.h"
#include "wvhashtable.h"
#include "wvstring.h"

// from stresshashtest.cc
// BEGIN first old-style definition
struct Intstr
{
    int i;
    WvString s;
    
    Intstr(int _i, WvStringParm _s)
        { i = _i; s = _s; }
};
DeclareWvDict(Intstr, WvString, s);
// END first old-style definition

WVTEST_MAIN("old-style")
{
    const unsigned size = 1000, elems = 10000;
    //free(malloc(1)); // enable electric fence
    
    IntstrDict d(size);
    unsigned count, total;
    bool add_passed = true, remove_passed = true, fast_iter_passed = true,
        slow_iter_passed = true;
    
    for (count = 0; count < elems; count++)
    {
#ifdef DEBUG_MESSAGES
        if (elems > 100 && !(count % (elems/20)))
        {
            printf("\rAdding %d%%", count / (elems/100));
            fflush(stdout);
        }
#endif
	d.add(new Intstr(count, count), true);
    }
    add_passed = d.count() == elems;
    WVPASS(add_passed);
    
    total = 0;
    for (count = 0; count < d.numslots; count++)
    {
	WvLink *head = &d.wvslots[count].head;
	if (!head->next)
	    total++;
    }
#ifdef DEBUG_MESSAGES
    printf("%d of %d empty slots in the table.\n", total, d.numslots);
    printf("Avg chain length: %d (best %d)\n",
	   elems / (d.numslots - total), elems / d.numslots);

    printf("Removing...");
    fflush(stdout);
#endif 
    
    for (count = 0; count < elems; count += 5)
    {
	if (!d[count])
            remove_passed = false;
	d.remove(d[count]);
    }
    
    WVPASS(remove_passed);
    WVPASS(d.count() == elems - elems/5);
    
    IntstrDict::Iter i(d);
    total = d.count();
    count = 0;
    
    for (i.rewind(); i.next(); )
    {
#ifdef DEBUG_MESSAGES
	if (total > 20 && !(count % (total/20)))
	{
	    printf("\rIterate %d%%", count / (total/100));
	    fflush(stdout);
	}
#endif
	if (!(i->s == WvString(i->i)))
            fast_iter_passed = false;
	count++;
    }
    
    WVPASS(fast_iter_passed);
    
    for (count = 0; count < total; count++)
    {
#ifdef DEBUG_MESSAGES
	if (total > 100 && !(count % (total/20)))
	{
	    printf("\rSlow Iterate %d%%", count / (total/100));
	    fflush(stdout);
	}
#endif
    if ((count%5) && !(d[count] && d[count]->s == WvString(d[count]->i)))
        slow_iter_passed = false;
    }

    WVPASS(slow_iter_passed);
}
