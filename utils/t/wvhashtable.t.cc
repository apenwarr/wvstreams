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

WVTEST_MAIN("hashtest.cc")
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
	   WvHash(x), WvHash(y), WvHash(z), WvHash((const char *)0),
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
    IntstrDict2 d(10);

    d.add(&a, false);
    d.add(&b, false);
    d.add(&c, false);
    printf("Dict Result: %p %p %p %p\n", d[a.i], d[b.i], d[7], d[10]);

    d.remove(&b);
    printf("Dict Result: %p %p %p %p\n", d[a.i], d[b.i], d[7], d[10]);

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

