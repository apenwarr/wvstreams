
#include "wvhashtable.h"
#include "wvstring.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

struct Intstr
{
    int i;
    WvString s;
    
    Intstr(int _i, const WvString &_s)
        { i = _i; s = _s; }
};

DeclareWvDict(Intstr, WvString, s);

const unsigned size = 1000, elems = 10000;

int main()
{
    //free(malloc(1)); // enable electric fence
    
    IntstrDict d(size);
    unsigned count, total;
    
    for (count = 0; count < elems; count++)
    {
	if (elems > 100 && !(count % (elems/20)))
	{
	    printf("\rAdding %d%%", count / (elems/100));
	    fflush(stdout);
	}
	d.add(new Intstr(count, count), true);
    }
    
    printf("\rAdded: total %d (should be %d)\n", d.count(), elems);
    
    total = 0;
    for (count = 0; count < d.numslots; count++)
    {
	WvLink *head = &d.slots[count].head;
	if (!head->next)
	    total++;
    }
    printf("%d of %d empty slots in the table.\n", total, d.numslots);
    printf("Avg chain length: %d (best %d)\n",
	   elems / (d.numslots - total), elems / d.numslots);
    

    printf("Removing...");
    fflush(stdout);
    
    for (count = 0; count < elems; count += 5)
    {
	assert(d[count]);
	d.remove(d[count]);
    }
    
    printf("\rRemoved.  New count: %d (should be %d)\n",
	   d.count(), elems - elems/5);
    
    IntstrDict::Iter i(d);
    total = d.count();
    count = 0;
    
    for (i.rewind(); i.next(); )
    {
	if (total > 20 && !(count % (total/20)))
	{
	    printf("\rIterate %d%%", count / (total/100));
	    fflush(stdout);
	}

	assert(i.data()->s == WvString(i.data()->i));
	count++;
    }
    
    printf("\rIterator okay.\n");

    
    for (count = 0; count < total; count++)
    {
	if (total > 100 && !(count % (total/20)))
	{
	    printf("\rSlow Iterate %d%%", count / (total/100));
	    fflush(stdout);
	}

	assert(!(count%5)
	       || (d[count] && d[count]->s == WvString(d[count]->i)));
    }
    
    printf("\rOne-by-one iterator okay.\n");
    
    return 0;
}
