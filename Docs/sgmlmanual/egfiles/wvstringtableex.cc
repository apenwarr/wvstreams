#include "wvstringtable.h"
#include "wvhashtable.h"
#include <stdio.h>

int main()
{
  WvStringTable t(10);
  // size: 10 elements
  // WvStringTable is essentially a WvHashTable



  WvString s("one"), s2("two"), s3("three");

  t.add(&s, false);
  t.add(&s2,false);
  t.add(&s3,false);
  // t.add("foo") is not allowed
  // refer to WvHashTable for more information

  printf("%s\n", t.join(",").cstr());
  //prints out: one,two,three


  printf("%s\n", t.join().cstr());
  // By default, t.join() is using " \t" as a delimiter
  // prints out: one         two     three


  t.zap();
  //erasing all contents of t


  t.split("a : b : c : d ", ":");

  printf("%s\n", t.join(",").cstr());
  // prints out: a , b , c , d


  t.split("x");
  t.split("y");
  t.split("z");

  printf("%s\n", t.join(",").cstr());
  // prints out: a , b , c , d ,x,y,z

  return 0;
}
