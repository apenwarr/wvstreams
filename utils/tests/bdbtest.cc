/*
 * Note: this test program should be exactly the same as the one in
 * qdbmtest.cc, because the APIs should be identical.
 */
#include "wvautoconf.h"
#if defined(WITH_QDBM) || defined(WITH_BDB)
#include "wvondiskhash.h"
#include "wvstringlist.h"

typedef WvOnDiskHash<WvString,WvString,WvBdbHash> StrStrMap;
typedef WvOnDiskHash<WvString,int,WvBdbHash> StrTimeMap;
typedef WvOnDiskHash<int,int,WvBdbHash> IntIntMap;

DeclareWvList(int);

// yes, I *am* crazy.
typedef WvOnDiskHash<WvList<WvString>,intList, WvBdbHash> ListMap;

int main()
{
    {
	StrStrMap ss("dbmfile");
	printf("\nList of strings:\n");
	{
	    StrStrMap::Iter i(ss);
            int guard = 0;
	    for (i.rewind(); i.next() && guard < 5; guard++)
		printf("'%s': '%s'\n", i.key().cstr(), i->cstr());
            if (guard == 5) printf("FAILED\n");
	}
	
	ss.add("hello", "world", true);
	ss.add("hello", "world2", true);
	ss.add("yellow", "5", true);
	if (ss.exists("appendable"))
	    ss.add("appendable",
		   ss["appendable"].append("-yak"), true);
	else
	    ss.add("appendable", "init");
	printf("appendable is now '%s'\n", ss["appendable"].cstr());

        printf("\nRestarting an iter:\n");
        {
            StrStrMap::Iter i(ss);
            int guard = 0;
            for (i.rewind(); i.next() && guard < 5; guard++)
            {
                printf("1 - '%s': '%s'\n", i.key().cstr(), i->cstr());
                if (i.key() == "hello")
                {
                    printf("Hit 'hello': modifying and starting over\n");
                    i().append("-yak");
                    i.save();
                    int guard2 = 0;
                    for (i.rewind(); i.next() && guard2 < 5; guard2++)
                        printf("1 - '%s': '%s'\n", i.key().cstr(), i->cstr());
                    if (guard == 5) guard2 = 5;
                    break;
                }
            }
            if (guard == 5) printf("FAILED\n");
        }

        printf("\nUnlinking 'hello' from an iter:\n");
        {
            StrStrMap::Iter i(ss);
            int guard = 0;
            for (i.rewind(); i.next() && guard < 5; guard++)
            {
		printf("1 - '%s': '%s'\n", i.key().cstr(), i->cstr());
                if (i.key() == "hello")
                    i.xunlink();
            }
            if (guard == 5) printf("FAILED\n");
            for (i.rewind(), guard = 0; i.next() && guard < 5; guard++)
		printf("2 - '%s': '%s'\n", i.key().cstr(), i->cstr());
            if (guard == 5) printf("FAILED\n");
        }
    }

    {
	StrTimeMap st;
        st.opendb("dbmfile2");
	printf("\nList of times:\n");
	{
	    StrTimeMap::Iter i(st);
            int guard = 0;
            for (i.rewind(); i.next() && guard < 5; guard++)
		printf("'%s': %d\n", i.key().cstr(), *i);
            if (guard == 5) printf("FAILED\n");
	}
	
	st.add("hello", 65, true);
	st.add("hello", 66, true);
	st.add("mellow", 97650, true);
	if (st.exists("addable"))
	    st.add("addable", st["addable"] + 5, true);
	else
	    st.add("addable", 7);
	printf("addable is now '%d'\n", st["addable"]);
    }
    
    {
	ListMap m("dbmfile3");
	
	// l1 is the same as l2; l3 is different.
	WvStringList l1; l1.split("big moose munchkin bunnies");
	WvStringList l2; l2.split("big   \nmoose\tmunchkin\n\nbunnies");
	WvStringList l3; l3.split("big   \nmoosemunchkin\n\nbunnies");
	printf("key elements: %d\n", l1.count());
	printf("key elements: %d\n", l2.count());
	printf("key elements: %d\n", l3.count());
	
	intList i1; i1.append(new int(7), true); i1.append(new int(8), true);
	intList i2; i2.append(new int(96), true);
	
	m.add(l1, i1, true);
	m.add(l2, i2, true);
	m.add(l3, i1, true);
	
	//for (int count = 0; count < 10000; count++)
	{
	    ListMap::Iter i(m);
            int guard = 0;
            for (i.rewind(); i.next() && guard < 5; guard++)
	    {
		WvList<WvString> &key = i.key();
		intList &data = i();
		
		printf("key(%d): ", key.count());
		{
		    WvList<WvString>::Iter ii(key);
		    for (ii.rewind(); ii.next(); )
			printf("'%s' ", ii->cstr());
		}
		printf("\n    ");
		{
		    intList::Iter ii(data);
		    for (ii.rewind(); ii.next(); )
			printf("%d ", ii());
		}
		printf("\n");
	    }
            if (guard == 5) printf("FAILED\n");
	}
    }

    {
        printf("\nTesting anonymous db:\n");
        bool ok = true;
        StrStrMap st;
        if (!st.isempty()) { ok = false; printf("Anon-db is NOT empty: FAILED\n"); }
        st.add("foo", "bar");
        if (st["foo"] != "bar") {
            ok = false;
            printf(WvString("Put in foo: bar; got out foo: %s: FAILED\n", st["foo"]));
        }
        if (ok) printf("Success!\n");
    }

    {
        printf("\nTesting iter:\n");
        IntIntMap m("ints");
        m.add(42,43,true);
        IntIntMap::Iter i(m);
        int guard = 0;
        for (i.rewind(); i.next() && guard < 5; guard++)
        {
            printf( "%d -> %d\n", i.key(), i() );
            i.save();
        }
        if (guard == 5) printf("FAILED\n");
    }
}
#else
int main(int argc, char *argv[])
{
  return 0;
}
#endif
