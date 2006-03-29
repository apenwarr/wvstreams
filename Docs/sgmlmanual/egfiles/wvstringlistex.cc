#include "wvstringlist.h"
#include "wvhashtable.h"
#include <stdio.h>

int main()
{
  WvStringList l;
  // WvStringList is essentially a WvHashTable

  WvString s("one"), s2("two"), s3("three"), foo("a : b : c : d");


  l.append(&s, false);
  l.append(&s2, false);
  l.append(&s3, false);

  WvStringList::Iter i(l);
  // iterator i can go through the list

  for (i.rewind(); i.next();)
    printf("The list: %s\n", i().cstr());

  l.zap();
  // clean the list

  l.split(foo, ": ");
  // split the variable foo with the delimiter ": " and append to the list

  for (i.rewind(); i.next();)
     printf("Split foo: %s\n", i().cstr());
  //prints:
  //Split foo: a
  //Split foo: b
  //Split foo: c
  //Split foo: d

  l.zap();
  l.split(foo, ": ", 2);
  // split the variable foo with the delimiter ": " and limit = 2
  // and append to the list

  for (i.rewind(); i.next();)
     printf("Split foo (2): %s\n", i().cstr());
  //prints:
  //Split foo (2): a
  //Split foo (2): b : c : d


  l.zap();
  l.split(foo, ": ", 3);
  // split the variable foo with the delimiter ": " and limit = 3
  // and append to the list

  for (i.rewind(); i.next();)
     printf("Split foo (3): %s\n", i().cstr());
  //prints:
  //Split foo (3): a
  //Split foo (3): b
  //Split foo (3): c : d


  /**************************************************
  Up until here, all is the same as WvStringTable
   Now we'll use popstr() and fill()
  ***************************************************/

  printf("Popping: %s\n", l.popstr().cstr());
  //prints:
  //Popping: a

  printf("Popping: %s\n", l.popstr().cstr());
  //prints:
  //Popping: b

  l.zap();

  char const *p = "hello";
  char const *p2 = "world";
  char const * const array[] = {p, p2, NULL};
  l.fill(array);

  printf("After fill: %s\n", l.join(",").cstr());
  //prints: After fill: hello

  l.zap();

  l.append(&s, false);
  l.append(&s2, false);
  l.append(&s3, false);
  l.fill(array);


  printf("After fill: %s\n", l.join(",").cstr());
  //prints: After fill: one,two,three,hello,world


  return 0;
}
