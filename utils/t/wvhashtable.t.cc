#include "wvtest.h"
#include "wvhashtable.h"
#include "wvstring.h"

struct Intstr
{
    int i;
    WvString s;
    
    Intstr(int _i, WvStringParm _s)
        { i = _i; s = _s; }
};
DeclareWvDict(Intstr, WvString, s);

DeclareWvTable(WvString);
DeclareWvDict2(IntstrDict2, Intstr, int, i);

class AutoFreeTest
{
public:
    AutoFreeTest()
        { fprintf (stderr, "AutoFreeTest created.\n"); }
    ~AutoFreeTest()
        { fprintf (stderr, "AutoFreeTest deleted.\n"); }
};

WVTEST_MAIN("WvHash(string) case insensitivity")
{
    WVPASS(WvHash("I'm a weasel") == WvHash("i'M a weAseL"));
    WVPASS(WvHash("a") == WvHash("A"));
}

 
WVTEST_MAIN("stresshash")
{
    const unsigned size = 1000, elems = 10000;
    //free(malloc(1)); // enable electric fence
    
    IntstrDict d(size);
    unsigned count, total;
    bool add_passed = true, remove_passed = true, fast_iter_passed = true,
        slow_iter_passed = true;
    
    for (count = 0; count < elems; count++)
    {
        if (elems > 100 && !(count % (elems/20)))
        {
            printf("\rAdding %d%%", count / (elems/100));
            fflush(stdout);
        }
	d.add(new Intstr(count, count), true);
    }
    add_passed = d.count() == elems;
    printf("\n");
    WVPASS(add_passed);
    
    total = 0;
    for (count = 0; count < d.numslots; count++)
    {
	if (d.wvslots[count].isempty())
	    total++;
    }
    
    printf("%d of %d empty slots in the table.\n", total, d.numslots);
    WVPASS(100*total/d.numslots < 20); // less than 20% empty
	   
    size_t avglength = elems / (d.numslots - total);
    size_t ideal = elems / d.numslots;
    printf("Avg chain length: %d (ideally %d)\n", avglength, ideal);

    printf("Removing...\n");
    fflush(stdout);
    
    for (count = 0; count < elems; count += 5)
    {
	if (!d[count])
            remove_passed = false;
	else
	    d.remove(d[count]);
    }
    
    WVPASS(remove_passed);
    WVPASS(d.count() == elems - elems/5);
    
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
	if (!(i->s == WvString(i->i)))
            fast_iter_passed = false;
	count++;
    }
    
    printf("\n");
    WVPASS(fast_iter_passed);
    
    for (count = 0; count < total; count++)
    {
	if (total > 100 && !(count % (total/20)))
	{
	    printf("\rSlow Iterate %d%%", count / (total/100));
	    fflush(stdout);
	}
	if ((count%5) && !(d[count] && d[count]->s == WvString(d[count]->i)))
	    slow_iter_passed = false;
    }

    printf("\n");
    WVPASS(slow_iter_passed);
}

// some things are commented out here because I believe the memory addresses are
// not absolute.. I'll admit I can be wrong, so modify if you wish
WVTEST_MAIN("hashtest.cc")
{
    WvString x("foo"), y("blue"), z("true");
    WvString x2("foo"), y2("blue"), z2("true"), xx("wuzzy");
    
    WvStringTable t(10);
    
    WVPASS(t.numslots == 15);
    
    t.add(&x, false);
    t.add(&y, false);
    t.add(&z, false);

    WVPASS(WvHash(x) != 0);
    WVPASS(WvHash(y) != 0);
    WVPASS(WvHash(z) != 0);
    WVPASS(WvHash((const char *)0) == 0);
    
    if (!WVPASS(&x == t[x2]))
        printf("   because [%p] != [%p]\n", &x, t[x2]);
    if (!WVPASS(&y == t[y2]))
        printf("   because [%p] != [%p]\n", &y, t[y2]);
    if (!WVPASS(&z == t[z2]))
        printf("   because [%p] != [%p]\n", &z, t[z2]);
    WVPASS(!t[xx]);
    
    if (!WVPASS(t.count() == 3))
        printf("   because [%d] != [3]\n", t.count());
    
    t.remove(&x);
    t.remove(&y2);
    WVPASS(!t[x2]);
    WVPASS(!t[y2]);
    WVPASS(!t[xx]);
    if (!WVPASS(t.count() == 1))
        printf("   because [%d] != [1]\n", t.count());
    
    WvStringTable::Iter i(t);
    for (i.rewind(); i.next(); )
        if (!WVPASS(!strcmp((const char *)i(), "true")))
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

WVTEST_MAIN("WvMap basics")
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

WVTEST_MAIN("WvMap iterators")
{
    char *strs[4] = {"death", "cow", "trek", "office"};
    WvMap<WvString, WvString> map(5);
    map.add(1, strs[1]);
    map.add(2, strs[2]);
    map.add(0, strs[0]);
    map.add(3, strs[3]);
    
    WvMap<WvString, WvString>::Iter iter(map);
    int i = 0;
    for (iter.rewind(); iter.next(); i++)
    {
	int i = iter->key.num();
	WVPASS(WvString(i) == iter->key);
	WVFAIL(i < 0);
	WVFAIL(i > 3);
	WVPASS(strs[i] == iter->data);
	strs[i] = NULL; // prevent duplicates
    }
    WVPASS(i == 4);
}

// Check that auto_free does nothing for objects
WVTEST_MAIN("WvMap auto_free")
{
    WvMap<WvString, WvString> objfreemap (5);
    objfreemap.add("foo", "bar", true);
    if (!WVPASS(*objfreemap.find("foo") == "bar"))
	printf("   because [%s] != \"bar\"\n", 
	       objfreemap.find("foo")->cstr());
    
    objfreemap.remove("foo");
    if (!WVFAIL(objfreemap.find("foo")))
	printf("   because objfreemap.find(\"foo\") == [%s]\n", 
	       objfreemap.find("foo")->cstr());
}

WVTEST_MAIN("WvMap auto_free 2")
{
    WvMap<WvString, AutoFreeTest*> freemap(5);
    freemap.add("moo", new AutoFreeTest(), true);
    WVPASS(freemap.find("moo"));
    freemap.remove("moo");
    WVFAIL(freemap.find("moo"));
}

// check if iterating confuses auto_free 
// like auto_ptrs in stdc++ containers
WVTEST_MAIN("WvMap auto_free and sorting")
{
    WvMap<WvString, AutoFreeTest*> freemap(5);
    freemap.add("meaw", new AutoFreeTest(), true);
    freemap.add("dog", new AutoFreeTest(), true);
    freemap.add("star", new AutoFreeTest(), true);
    freemap.add("star", new AutoFreeTest(), true);
    
    int i = 0;
    WvMap<WvString, AutoFreeTest*>::Iter j(freemap);
    for (j.rewind(); j.next(); i++)
	WVPASS(strlen(j->key) == 3 || strlen(j->key) == 4);
    WVPASS(i == 4);
}

WVTEST_MAIN("WvMap zapping")
{
    /*** FAILURE: This doesn't pass as of 2004-07-13.  See BUGZID: 6281 ***/
    /*
    WvMap<WvString, WvString> zapmap(5);
    zapmap.add("Febtober", "Months that start with 'Feb'");
    zapmap.add("Febturday", "Months that start with 'Feb'");

    WVPASS(zapmap.exists("Febtober"));
    zapmap.zap();
    WVFAIL(zapmap.exists("Febtober"));

    WvMap<WvString, WvString*> ptrmap(5);

    ptrmap.set("Smarch", new WvString("Mapril"));
    WVPASS(ptrmap.exists("Smarch"));
    ptrmap.zap();
    WVFAIL(listmap.exists("Smarch"));
    */

}

WVTEST_MAIN("WvMap removing")
{
    WvMap<WvString, WvString> remmap(5);
    remmap.add("ruff", "This is the sound a dog makes");
    WVPASS(remmap.exists("ruff"));
    remmap.remove("ruff");
    WVFAIL(remmap.exists("ruff"));

    /*** FAILURE: This doesn't pass as of 2004-07-15.  See BUGZID: 6281 ***/
    /*
    WvMap<WvString, WvString*> remmap2(5);
    remmap2.add("moo", new WvString("This is the sound a dog makes"));
    WVPASS(remmap2.exists("moo"));
    remmap.remove("moo");
    WVFAIL(remmap.exists("moo"));
    */
}

