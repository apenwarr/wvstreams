
#include "wvlinklist.h"
#include "wvstring.h"
#include <stdio.h>

DeclareWvList(WvString);
DeclareWvList(int);

int main()
{
    WvString x("foo"), y("blue"), z("true");

    WvStringList l;
    WvStringList::Iter i(l);
    
    l.append(&x, false);
    l.append(&y, false);
    l.append(&z, false);

    for (i.rewind(); i.next();)
	printf("Thingy: %s\n", i.data()->str);

    int a=5, b=6;
    intList il;
    intList::Iter ii(il);
    
    il.prepend(&a, false);
    il.prepend(&b, false);
    
    ii.rewind();
    while (ii.next())
	printf("Dingy: %d\n", *ii.data());
    
    return 0;
}
