/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-1999 Worldvisions Computer Technology, Inc.
 *
 * WvHashTable test program.  (One) correct output:
 *
 *    Hash table size: 15
 *    String hash test:
 *             000019ef 000132a5 000a4aa5 00000000
 *             f3ad1ec0 f3ad1ec0 f3ad5ac0 f3ad1fd0
 *    Correct answers are: 0xbffffaf0 0xbffffae8 0xbffffae0 (0xbffffac0)
 *    
 *    Result: 0xbffffaf0 0xbffffae8 0xbffffae0 (nil)
 *    Full(3) contents: 0x804f167(blue) 0x804f163(foo) 0x804f16c(true)
 *    
 *    Result: (nil) (nil) 0xbffffae0 (nil)
 *    Full(1) contents: 0x804f16c(true)
 *    
 *    Dict Result: 0xbffffaa0 0xbffffa8c 0xbffffa80 (nil)
 *    Dict Result: 0xbffffaa0 (nil) 0xbffffa80 (nil)
 */

#include "wvhashtable.h"
#include "wvstring.h"
#include <stdio.h>

DeclareWvTable(WvString);

struct Intstr
{
    int i;
    WvString s;
    
    Intstr(int _i, const WvString &_s)
        { i = _i; s = _s; }
};

DeclareWvDict(Intstr, int, i);

int main()
{
    WvString x("foo"), y("blue"), z("true");
    WvString x2("foo"), y2("blue"), z2("true"), xx("wuzzy");
    
    WvStringTable t(10);
    
    printf("Hash table size: %d\n", t.numslots);
    
    t.add(&x, false);
    t.add(&y, false);
    t.add(&z, false);
    
    printf("String hash test: \n\t %08x %08x %08x %08x \n"
	   "\t %08x %08x %08x %08x\n",
	   WvHash(x), WvHash(y), WvHash(z), WvHash(NULL),
	   WvHash("fuzzy wuzzy buzzy foo"), WvHash("FUZZY wuzzy BUZZY foo"),
	   WvHash("fuzzy wuzzy buzzy woo"),
	   WvHash("wuzzy wuzzy buzzy foo"));

    printf("Correct answers are: %p %p %p (%p)\n\n", &x, &y, &z, &xx);
    printf("Result: %p %p %p %p\n", t[x2], t[y2], t[z2], t[xx]);
    
    WvStringTable::Iter i(t);
    printf("Full(%d) contents: ", t.count());
    for (i.rewind(); i.next(); )
	printf("%p(%s) ", (const char *)i(), (const char *)i());
    printf("\n\n");
    
    t.remove(&x);
    t.remove(&y2);
    printf("Result: %p %p %p %p\n", t[x2], t[y2], t[z2], t[xx]);
    printf("Full(%d) contents: ", t.count());
    for (i.rewind(); i.next(); )
	printf("%p(%s) ", (const char *)i(), (const char *)i());
    printf("\n\n");
    
    
    Intstr a(5, "big"), b(6, "whistle"), c(7, "money");
    IntstrDict d(10);

    d.add(&a, false);
    d.add(&b, false);
    d.add(&c, false);
    printf("Dict Result: %p %p %p %p\n", d[a.i], d[b.i], d[7], d[10]);

    d.remove(&b);
    printf("Dict Result: %p %p %p %p\n", d[a.i], d[b.i], d[7], d[10]);

    return 0;
}
