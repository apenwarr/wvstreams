#include "wvtest.h"
#include "wvhashtable.h"
#include "wvstring.h"

// BEGIN stresshashtest.cc definition
struct Intstr
{
    int i;
    WvString s;
    
    Intstr(int _i, WvStringParm _s)
        { i = _i; s = _s; }
};
DeclareWvDict(Intstr, WvString, s);
// END stresshashtest.cc definition

// BEGIN hashtest.cc definiton
DeclareWvTable(WvString);
DeclareWvDict2(IntstrDict2, Intstr, int, i);
// END hashtest.cc definition
// 
// START maptest.cc definition
class AutoFreeTest
{
public:
    AutoFreeTest ()
    {
//        fprintf (stderr, "AutoFreeTest created.\n");
    };
    ~AutoFreeTest ()
    {
//        fprintf (stderr, "AutoFreeTest deleted.\n");
    };
};
// END maptest.cc definition 
 
WVTEST_MAIN("stresshashtest.cc")
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

// some things are commented out here because I believe the memory addresses are
// not absoulte.. I'll admit I can be wrong, so modify if you wish
WVTEST_MAIN("hashtest.cc")
{
    WvString x("foo"), y("blue"), z("true");
    WvString x2("foo"), y2("blue"), z2("true"), xx("wuzzy");
    
    WvStringTable t(10);
    
    WVPASS(t.numslots == 15);
    
    t.add(&x, false);
    t.add(&y, false);
    t.add(&z, false);
    
    if (!WVPASS(WvHash(x) == 0x000019ef))
        printf("   because [%08x] != [000019ef]\n", WvHash(x));
    if (!WVPASS(WvHash(y) == 0x000132a5))
        printf("   because [%08x] != [000132a5]\n", WvHash(y));
    if (!WVPASS(WvHash(z) == 0x000a4aa5))
        printf("   because [%08x] != [000a4aa5]\n", WvHash(z));
    if (!WVPASS(WvHash((const char*)0) == 0x0000000))
        printf("   because [%08x] != [00000000]\n", WvHash(x));
    
    if (!WVPASS(WvHash("fuzzy wuzzy buzzy foo") == 0xf3ad1ec0))
        printf("   because [%08x] != [f3ad1ec0]\n", 
                WvHash("fuzzy wuzzy buzzy foo"));
    if (!WVPASS(WvHash("FUZZY wuzzy BUZZY foo") == 0xf3ad1ec0))
        printf("   because [%08x] != [f3ad1ec0]\n", 
                WvHash("FUZZY wuzzy BUZZY foo"));
    if (!WVPASS(WvHash("fuzzy wuzzy buzzy woo") == 0xf3ad5ac0))
        printf("   because [%08x] != [f3ad5ac0]\n", 
                WvHash("fuzzy wuzzy buzzy woo"));
    if (!WVPASS(WvHash("wuzzy wuzzy buzzy foo") == 0xf3ad1fd0))
        printf("   because [%08x] != [f3ad1fd0]\n", 
                WvHash("wuzzy wuzzy buzzy foo"));
    
    if (!WVPASS(&x == t[x2]))
        printf("   because [%p] != [%p]\n", &x, t[x2]);
    if (!WVPASS(&y == t[y2]))
        printf("   because [%p] != [%p]\n", &y, t[y2]);
    if (!WVPASS(&z == t[z2]))
        printf("   because [%p] != [%p]\n", &z, t[z2]);
//    WVPASS(&xx == 0xbffffac0); //why does it forbid this comparison?
    if (!WVPASS(t[xx] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", t[xx]);
    
    WvStringTable::Iter i(t);
//    unsigned long pdesired[3] = {0x804ca58, 0x804ca40, 0x804ca70};
    int j = 0;
    char *cdesired[3] = {"blue", "foo", "true"};
    
    if (!WVPASS(t.count() == 3))
        printf("   because [%d] != [3]\n", t.count());
    for (i.rewind(); i.next(); )
    {
/*        if (!WVPASS((unsigned long)&i() == pdesired[j]))
            printf("   because [%p] != [0x%08x]\n", (const char *)i(),
                    pdesired[j]);*/
        if (!WVFAIL(strcmp((const char *)i(), cdesired[j])))
            printf("   because [%s] != [%s]\n", (const char *)i(),
                    cdesired[j]);
        j++;
    }
    
    t.remove(&x);
    t.remove(&y2);
    if (!WVPASS(t[x2] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", t[x2]);
    if (!WVPASS(t[y2] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", t[y2]);
/*    if (!WVPASS((unsigned)t[z2] == 0xbffffb00))
        printf("   because [%p] != [0xbffffb00]\n", t[z2]);*/
    if (!WVPASS(t[xx] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", t[xx]);
    if (!WVPASS(t.count() == 1))
        printf("   because [%d] != [1]\n", t.count());
    
    for (i.rewind(); i.next(); )
        if (!WVFAIL(strcmp((const char *)i(), "true")))
            printf("   because [%s] != [true]\n", (const char *)i());
    
    Intstr a(5, "big"), b(6, "whistle"), c(7, "money");
    IntstrDict2 d(10);

    d.add(&a, false);
    d.add(&b, false);
    d.add(&c, false);
/*    if (!WVPASS((unsigned)d[a.i] == 0xbffffabc))
        printf("   because [%p] != [0xbffffabc]\n", d[a.i]);
    if (!WVPASS((unsigned)d[b.i] == 0xbffffaa8))
        printf("   because [%p] != [0xbffffaa8]\n", d[b.i]);
    if (!WVPASS((unsigned)d[7] == 0xbffffa9c))
        printf("   because [%p] != [0xbffffa9c]\n", d[7]);*/
    if (!WVPASS((unsigned)d[10] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", d[10]);

    d.remove(&b);
/*    if (!WVPASS((unsigned)d[a.i] == 0xbffffabc))
        printf("   because [%p] != [0xbffffabc]\n", d[a.i]);*/
/*    if (!WVPASS((unsigned)d[b.i] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", d[b.i]);
    if (!WVPASS((unsigned)d[7] == 0xbffffa9c))
        printf("   because [%p] != [0xbffffa9c]\n", d[7]);*/
    if (!WVPASS((unsigned)d[10] == 0x00000000))
        printf("   because [%p] != [0x00000000]\n", d[10]);
}

WVTEST_MAIN("maptest.cc")
{
    {
        WvMap<WvString, WvString> map(5);
        map.add ("foo", "bar");
        if (!WVPASS(*map.find("foo") == "bar"))
            printf("   because [%s] != \"bar\"\n", map.find("foo")->cstr());
        map.remove("foo");
        if (!WVFAIL(map.find("foo")))
            printf("   because map.find(\"foo\") == [%s]\n", 
                    map.find("foo")->cstr());
    }

    // Iterator test
    {
        WvMap<WvString, WvString> map(5);
        map.add("meaw", "death");
        map.add("dog", "cow");
        map.add("star", "trek");
        map.add("star", "office");

        WvMap<WvString, WvString>::Iter iter(map);
        char *key[4] = {"meaw", "star", "star", "dog"};
        char *data[4] = {"death", "trek", "office", "cow"};
        int i = 0;
        for (iter.rewind(); iter.next(); i++)
        {
            if (!WVPASS(key[i] == iter->key))
                printf("   because [%s] != [%s]\n", key[i], iter->key.cstr());
            if (!WVPASS(data[i] == iter->data))
                printf("   because [%s] != [%s]\n", data[i], iter->data.cstr());
        }
    }

    // Check that auto_free does nothing for objects
    {
        WvMap<WvString, WvString> objfreemap (5);
        objfreemap.add ("foo", "bar", true);
        if (!WVPASS(*objfreemap.find("foo") == "bar"))
            printf("   because [%s] != \"bar\"\n", 
                    objfreemap.find("foo")->cstr());
        
        objfreemap.remove("foo");
        if (!WVFAIL(objfreemap.find("foo")))
            printf("   because objfreemap.find(\"foo\") == [%s]\n", 
                    objfreemap.find("foo")->cstr());
    }
    {
        WvMap<WvString, AutoFreeTest*> freemap(5);
        freemap.add("moo", new AutoFreeTest(), true);
        WVPASS(freemap.find("moo"));
        freemap.remove("moo");
        WVFAIL(freemap.find("moo"));
    }

    // check if sorting confuses auto_free 
    // a'la auto_ptr's in standard containers
    {
        WvMap<WvString, AutoFreeTest*> freemap(5);
        char *keys[4] = {"meaw", "star", "star", "dog"};
        freemap.add("meaw", new AutoFreeTest(), true);
        freemap.add("dog", new AutoFreeTest(), true);
        freemap.add("star", new AutoFreeTest(), true);
        freemap.add("star", new AutoFreeTest(), true);

        WvMap<WvString, AutoFreeTest*>::Iter j(freemap);
        int i = 0;
        for (j.rewind(); j.next(); i++)
        {
            if (!WVPASS(j->key == keys[i]))
                printf("   because [%s] != [%s]\n", j->key.cstr(), keys[i]);
        }
    }
}

