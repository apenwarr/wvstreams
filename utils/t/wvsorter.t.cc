#include "wvtest.h"
#include "wvstring.h"
#include "wvlinklist.h"
#include "wvhashtable.h"
#include "wvscatterhash.h"

// BEGIN first old-style definition
DeclareWvList(WvString);
DeclareWvTable(WvString);
DeclareWvScatterTable2(WvStringTable2, WvString);

int apples_to_oranges(const WvString *a, const WvString *b)
{
    return strcmp(*a, *b);
}

int oranges_to_apples(const WvString *a, const WvString *b)
{
    return -strcmp(*a, *b);
}
// END first old-style definition

WVTEST_MAIN("old-style")
{
    free(malloc(1));
    char *frontwards[10] = {"eight", "five", "four", "nine", "one", "seven",
        "six", "ten", "three", "two"};
    char *backwards[10] = {"two", "three", "ten", "six", "seven", "one", 
        "nine", "four", "five", "eight"};
        
    // sorted linked list
    {
        char *unsorted[10] = {"one", "two", "three", "four", "five", "six",
            "seven", "eight", "nine", "ten"};
	printf("\nLinked list sorter test:\n");
	WvStringList l;
        for (int i = 0; i < 10; i++)
            l.append(new WvString(unsorted[i]), true);
	
	//Frontwards:
	{
	    WvStringList::Sorter s(l, apples_to_oranges);
            int i = 0;
	    for(s.rewind(); s.next(); i++)
                if (!WVFAIL(strcmp((const char*) s(), frontwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            frontwards[i]);
	}
	
	//Unsorted:
	{
	    WvStringList::Iter i(l);
            int j = 0;
	    for(i.rewind(); i.next(); j++)
                 if (!WVFAIL(strcmp((const char *) i(), unsorted[j])))
                     printf("   because [%s] != [%s]\n", (const char*) i(),
                             unsorted[j]);
	}
	
	//Backwards:
	{
	    WvStringList::Sorter s(l, oranges_to_apples);
            int i = 0;
	    for(s.rewind(); s.next(); i++)
                if (!WVFAIL(strcmp((const char*) s(), backwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            backwards[i]);
	}
    }

    // sorted hash table
    {
        char *unsorted[10] = {"one", "two", "three", "four", "seven", "eight",
            "five", "six", "nine", "ten"};
	WvStringTable t(3);
        for (int i = 0; i < 10; i++)
	    t.add(new WvString(unsorted[i]), true);
	
	//Frontwards:
	{
	    WvStringTable::Sorter s(t, apples_to_oranges);
            int i = 0;
	    for(s.rewind(); s.next();i++)
                if (!WVFAIL(strcmp((const char*) s(), frontwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            frontwards[i]);            
	}
	
	//Unsorted: 
        {
            int j = 0;
	    WvStringTable::Iter i(t);
	    for(i.rewind(); i.next(); j++)
                 if (!WVFAIL(strcmp((const char *) i(), unsorted[j])))
                     printf("   because [%s] != [%s]\n", (const char*) i(),
                             unsorted[j]);
	}
	
	//Backwards:
	{
	    WvStringTable::Sorter s(t, oranges_to_apples);
            int i = 0;
	    for(s.rewind(); s.next(); i++)
                if (!WVFAIL(strcmp((const char*) s(), backwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            backwards[i]);            
	}
    }
    
    // sorted scatter hash table
    {
        char *unsorted[10] = {"one", "ten", "seven", "five", "nine", "eight",
            "six", "three", "two", "four"};
	WvStringTable2 t(3);
        for (int i = 0; i < 10; i++)
            t.add(new WvString(unsorted[i]), true);
        
	//Frontwards:
	{
	    WvStringTable2::Sorter s(t, apples_to_oranges);
            int i = 0;
	    for(s.rewind(); s.next();i++)
                if (!WVFAIL(strcmp((const char*) s(), frontwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            frontwards[i]);            
	}
	
	//Unsorted: 
        {
            int j = 0;
	    WvStringTable2::Iter i(t);
	    for(i.rewind(); i.next(); j++)
                 if (!WVFAIL(strcmp((const char *) i(), unsorted[j])))
                     printf("   because [%s] != [%s]\n", (const char*) i(),
                             unsorted[j]);
	}
	
	//Backwards:
	{
	    WvStringTable2::Sorter s(t, oranges_to_apples);
            int i = 0;
	    for(s.rewind(); s.next(); i++)
                if (!WVFAIL(strcmp((const char*) s(), backwards[i])))
                    printf("   because [%s] != [%s]\n", (const char*) s(),
                            backwards[i]);            
	}	
    }
}
