#include "wvgdbmhash.h"
#include "wvstringlist.h"

typedef WvGdbmHash<WvString,WvString> StrStrMap;
typedef WvGdbmHash<WvString,int> StrTimeMap;

DeclareWvList(int);

// yes, I *am* crazy.
typedef WvGdbmHash<WvList<WvString>,intList> ListMap;

int main()
{
    {
	StrStrMap ss("dbmfile");
	printf("\nList of strings:\n");
	{
	    StrStrMap::Iter i(ss);
	    for (i.rewind(); i.next(); )
		printf("'%s': '%s'\n", i.key().cstr(), i->cstr());
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
    }

    {
	StrTimeMap st("dbmfile2");
	printf("\nList of times:\n");
	{
	    StrTimeMap::Iter i(st);
	    for (i.rewind(); i.next(); )
		printf("'%s': %d\n", i.key().cstr(), *i);
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
	    for (i.rewind(); i.next(); )
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
	}
    }
}
