#include "wvtest.h"
#include "wvstringlist.h"
#include "wvlinklist.h"

DeclareWvList(int);
DeclareWvList2(numberList, int);
/**
 * I've only tested both Declares for basic functionality, they really shouldn't
 * ever be inconsistent, and it'd be a pretty major bug if they were, if that's
 * ever a bug, write a unit test for it.
 */
WVTEST_MAIN("basic")
{
    // test zap() and autofree()
    {
        // both lists should perform identically
        intList l1;
        numberList l2;
        bool datafreed = false;

        int *list[10];
        for (int i = 10; i < 20; i++)
            list[i - 10] = new int(i);

        // add 20 ints to the list
        for (int i = 0; i < 10; i++)
        {
            l1.add(new int(i), true);
            l2.add(new int(i), true);
        }
        for (int i = 10; i < 20; i++)
        {
            l1.add(list[i - 10], false);
            l2.add(list[i - 10], false);
        }
        
        WVFAIL(l1.isempty());
        WVFAIL(l2.isempty());
        l1.zap();
        l2.zap();
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
        
        // test that non-autofreed were left alone
        for (int i = 10; i < 20; i++)
        {
            if (list[i - 10] == NULL)
                datafreed = true;
        }
        WVFAIL(datafreed);
        for (int i = 0; i < 10; i++)
        {
            delete list[i];
        }
    }
    
    // test add(), first() and unlink_first()
    {
        // both lists should perform identically
        intList l1;
        numberList l2;
        bool datamatchl1 = true, datamatchl2 = true;
        
        // construct a list using add and test isempty() while at it
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
        WVPASS(l1.count() == 0);
        WVPASS(l2.count() == 0);
        for (int i = 0; i < 100; i++)
        {
            l1.add(new int(i), true);
            l2.add(new int(i), true);
        }
        WVPASS(l1.count() == 100);
        WVPASS(l2.count() == 100);
        WVFAIL(l1.isempty());
        WVFAIL(l2.isempty());

        // deconstruct the list testing first() and test isempty() again
        for (int i = 0; i < 100; i++)
        {
            if (*l1.first() != i)
                datamatchl1 = false;
            if (*l2.first() != i)
                datamatchl2 = false;
            l1.unlink_first();
            l2.unlink_first();
        }
        WVPASS(datamatchl1);
        WVPASS(datamatchl2);
        // should be empty again now
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
    }

    // test append(), last(), and unlink()
    {
        // both lists should perform identically
        intList l1;
        numberList l2;
        int *last1, *last2;
        bool datamatchl1 = true, datamatchl2 = true;    

        // construct a list using append() (should be same as add)
        datamatchl1 = datamatchl2 = true;
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
        for (int i = 0; i < 100; i++)
        {
            l1.append(new int(i), true);
            l2.append(new int(i), true);
        }
        WVPASS(l1.count() == 100);
        WVPASS(l2.count() == 100);
        WVFAIL(l1.isempty());
        WVFAIL(l2.isempty());

        // deconstruct the list testing last() and test isempty() again
        for (int i = 99; i >= 0; i--)
        {
            last1 = l1.last();
            last2 = l2.last();
            if (*last1 != i)
                datamatchl1 = false;
            if (*last2 != i)
                datamatchl2 = false;
            l1.unlink(last1);
            l2.unlink(last2);
        }
        WVPASS(datamatchl1);
        WVPASS(datamatchl2);
        // should be empty again now
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
    }
    
    // test prepend(), last(), unlink() and isempty()
    {
        // both lists should perform identically
        intList l1;
        numberList l2;
        int *last1, *last2;
        bool datamatchl1 = true, datamatchl2 = true;    
        
        // construct a list using prepend()
        datamatchl1 = datamatchl2 = true;
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
        for (int i = 0; i < 100; i++)
        {
            l1.prepend(new int(i), true);
            l2.prepend(new int(i), true);
        }
        WVPASS(l1.count() == 100);
        WVPASS(l2.count() == 100);
        WVFAIL(l1.isempty());
        WVFAIL(l2.isempty());

        // deconstruct the list testing last() and test isempty() again
        for (int i = 99; i >= 0; i--)
        {
            last1 = l1.first();
            last2 = l2.first();
            if (*last1 != i)
                datamatchl1 = false;
            if (*last2 != i)
                datamatchl2 = false;
            l1.unlink(last1);
            l2.unlink(last2);
        }
        WVPASS(datamatchl1);
        WVPASS(datamatchl2);
        // should be empty again now
        WVPASS(l1.isempty());
        WVPASS(l2.isempty());
    }

    // test iterator traversal
    {
        intList l1;
        for (int i = 0; i < 20; i++)
        {
            l1.add(new int(i), true);
        }
        
        intList::Iter i(l1);
        int curr = 0;
        bool iterMatches = true;
        for (i.rewind(); i.next(); curr++)
        {
            if (*i.ptr() != curr)
                iterMatches = false;
        }
        WVPASS(iterMatches);
    }
    
    // test deleting while iterating
    {
        intList l1;
        for (int i = 0; i < 20; i++)
        {
            l1.add(new int(i), true);
        }
        
        intList::Iter i(l1);
        for (i.rewind(); i.next(); )
        {
            i.xunlink();
        }
    }
}

WVTEST_MAIN("listtest.cc")
{
    WvString x("foo"), y("blue"), z("true"), bob("Foo: bar: baz: bob");

    WvStringList l;
    WvStringList::Iter i(l);
    
    l.append(&x, false);
    l.append(&y, false);
    l.append(&z, false);
    
    const char *out1[3] = {"foo", "blue", "true"};
    int j = 0;
    for (i.rewind(); i.next(); j++)
        if (!WVPASS(i() == out1[j]))
            printf("   because [%s] != [%s]\n", i().cstr(), out1[j]);

    const char *out2[4] = {"Foo", "bar", "baz", "bob"};
    j = 0;
    l.zap();
    l.split(bob, ": ");
    for (i.rewind(); i.next(); j++)
        if (!WVPASS(i() == out2[j]))
            printf("   because [%s] != [%s]\n", i().cstr(), out2[j]);

    const char *out3[2] = {"Foo", "bar: baz: bob"};
    j = 0;
    l.zap();
    l.split(bob, ": ", 2);
    for (i.rewind(); i.next(); j++)
        if (!WVPASS(i() == out3[j]))
            printf("   because [%s] != [%s]\n", i().cstr(), out3[j]);

    const char *out4[3] = {"Foo", "bar", "baz: bob"};
    j = 0;
    l.zap();
    l.split(bob, ": ", 3);
    for (i.rewind(); i.next(); j++)
        if (!WVPASS(i() == out4[j]))
            printf("   because [%s] != [%s]\n", i().cstr(), out4[j]);

    int a=5, b=6;
    int out5[2] = {6, 5};
    intList il;
    intList::Iter ii(il);
    
    il.prepend(&a, false);
    il.prepend(&b, false);
    
    ii.rewind();
    j = 0;
    while (ii.next())
    {
        if (!WVPASS(ii() == out5[j]))
            printf("   because [%d] != [%d]\n", ii(), out5[j]);
        j++;
    }
}
