/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for sorted iterators on lists.
 *
 * Correct output:
 *      Frontwards: eight five four nine one seven six ten three two 
 *      Unsorted:   one two three four five six seven eight nine ten 
 *      Backwards:  two three ten six seven one nine four five eight 
 *
 */

#include <stdio.h>
#include "wvstring.h"
#include "wvlinklist.h"
#include "wvhashtable.h"
#include "wvsorter.h"

DeclareWvList(WvString)
DeclareWvTable(WvString)

int apples_to_oranges(const WvString *a, const WvString *b)
{
    return strcmp(*a, *b);
}

int oranges_to_apples(const WvString *a, const WvString *b)
{
    return -strcmp(*a, *b);
}


int main()
{
    free(malloc(1));

    // sorted linked list
    printf("\nLinked list sorter test:\n");
    WvStringList l;
    l.append(new WvString("one"), true);
    l.append(new WvString("two"), true);
    l.append(new WvString("three"), true);
    l.append(new WvString("four"), true);
    l.append(new WvString("five"), true);
    l.append(new WvString("six"), true);
    l.append(new WvString("seven"), true);
    l.append(new WvString("eight"), true);
    l.append(new WvString("nine"), true);
    l.append(new WvString("ten"), true);

    printf("Frontwards: ");
    {
        WvStringList::Sorter s(l, apples_to_oranges);
        for(s.rewind(); s.next();)
            printf("%s ", (const char *) s());
    }

    printf("\nUnsorted:   ");
    {
        WvStringList::Iter i(l);
        for(i.rewind(); i.next();)
            printf("%s ", (const char *) i());
    }

    printf("\nBackwards:  ");
    {
        WvStringList::Sorter s(l, oranges_to_apples);
        for(s.rewind(); s.next();)
            printf("%s ", (const char *) s());
    }

    // sorted hash table
    printf("\n\nHash table sorter test:\n");
    WvStringTable t(3);
    t.add(new WvString("one"), true);
    t.add(new WvString("two"), true);
    t.add(new WvString("three"), true);
    t.add(new WvString("four"), true);
    t.add(new WvString("five"), true);
    t.add(new WvString("six"), true);
    t.add(new WvString("seven"), true);
    t.add(new WvString("eight"), true);
    t.add(new WvString("nine"), true);
    t.add(new WvString("ten"), true);

    printf("Frontwards: ");
    {
        WvStringTable::Sorter s(t, apples_to_oranges);
        for (s.rewind(); s.next(); )
            printf("%s ", (const char *) s());
    }

    printf("\nUnsorted:   ");
    {
        WvStringTable::Iter i(t);
        for (i.rewind(); i.next(); )
            printf("%s ", (const char *) i());
    }

    printf("\nBackwards:  ");
    {
        WvStringTable::Sorter s(t, oranges_to_apples);
        for (s.rewind(); s.next(); )
            printf("%s ", (const char *) s());
    }

    printf("\n");

    return 0;
}
