/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvHashTable sample program.
 * Suppose you are learning a new language that is alphabetical and you want
 * to register all the words you added to your vocabulary in that
 * language. So you will need a dictionary where you can look up on words
 * that you have learned.
 * Assuming that you will not forget any word you had previously learned,
 * this dictionary shall not contain repetitive words.
 */

#include "wvhashtable.h"
#include "wvstring.h"
#include "wvlinklist.h"
#include <stdio.h>

// Declare a HashTable class that handles WvString data types
DeclareWvTable(WvString);

/*this subfunction ascending is used for sorting*/
int ascending(const WvString *a, const WvString *b)
{
    return strncasecmp(*a, *b, strlen(a->cstr()));
}


int main()
{
     // This is a dictionary that can contain at most 10 words
     WvStringTable t(100);

     // Here's a list of new words in your vocabulary
     WvString s1("aussi"), s2("Bonjour"), s3("comment");
     WvString s4("demain"), s5("non"), s6("oui");
     WvString s7("matin"), s8("bonsoir"), s9("bien");
     WvString s10("depanneur");

     // Add the list of new words to the dictionary
     // false = do not autofree the WvString
     t.add(&s1, false); t.add(&s2, false), t.add(&s3, false);
     t.add(&s4, false); t.add(&s5, false), t.add(&s6, false);
     t.add(&s7, false); t.add(&s8, false), t.add(&s9, false), t.add(&s10, false);

     // Using an iterator, we can print out the entire content of the hashtable
     printf("What words do we have in the dictionary?\n");
     WvStringTable::Iter i(t);
     for( i.rewind(); i.next(); )
     {
	 printf("%s\n", i->cstr() );
     }

     printf("There are %d words stored in the dictionary so far.\n", t.count());

     // To look up words in the dictionary, put the WvString data inside the []
     // and do the following to print it out

     WvString sample1("Bonjour");
     printf("Is 'Bonjour' in the dictionary? %s\n", t[sample1]?"Yes":"No");
     WvString sample2("Salut");
     printf("Is 'Salut' in the dictionary? %s\n", t[sample2]?"Yes":"No");

     // To remove a word from the dictionary
     // For example, if you want to remove the word "aussi" in your dictionary
     t.remove(&s1);

     // Then print out the list of words in the dictionary again
     printf("Modified List:\n");
     for( i.rewind(); i.next(); )
     {
	 printf("%s\n", i->cstr() );
     }

     WvStringTable::Sorter s(t,ascending);
     printf("Sorted modified List:\n");
     for( s.rewind(); s.next(); )
     {
	 printf("%s\n", s->cstr() );
     }


     // You can empty the entire dictionary by doing this:
     t.zap();

     // Then print out the list of words in the dictionary again
     printf("Empty List:\n");
     for( i.rewind(); i.next(); )
     {
	 printf("%s\n", i->cstr() );
     }

}

