/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvStringList test program.  Correct results:
 * 	Thingy: foo
 * 	Thingy: blue
 * 	Thingy: true
 * 	Dingy: 6
 * 	Dingy: 5
 */

#include "wvlinklist.h"
#include "wvstringlist.h"
#include <stdio.h>

DeclareWvList(int);

int main()
{
    WvString x("foo"), y("blue"), z("true"), bob("Foo: bar: baz: bob");

    WvStringList l;
    WvStringList::Iter i(l);
    
    l.append(&x, false);
    l.append(&y, false);
    l.append(&z, false);

    for (i.rewind(); i.next();)
	printf("Thingy: %s\n", (const char *)(WvString&)i);

    l.zap();
    l.split(bob, ": ");
    for (i.rewind(); i.next();)
	printf("Stingy: %s\n", (const char *)(WvString&)i);

    l.zap();
    l.split(bob, ": ", 2);
    for (i.rewind(); i.next();)
	printf("Stingy(2): %s\n", (const char *)(WvString&)i);

    l.zap();
    l.split(bob, ": ", 3);
    for (i.rewind(); i.next();)
	printf("Stingy(3): %s\n", (const char *)(WvString&)i);

    int a=5, b=6;
    intList il;
    intList::Iter ii(il);
    
    il.prepend(&a, false);
    il.prepend(&b, false);
    
    ii.rewind();
    while (ii.next())
	printf("Dingy: %d\n", ii());

    return 0;
}
