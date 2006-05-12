#include "wvstringlist.h"
#include "wvhashtable.h"
#include <stdio.h>

DeclareWvList(int);
//Declaration of list of integers

int main()
{

  int a=5, b=6;

  intList l; //the list


  intList::Iter i(l); //the iterator

  l.add(&a, false);
  l.add(&b, false);

  for (i.rewind(); (i.next()) ; )
    printf("Foo: %d\n", i());

  //prints:
  //Foo: 5
  //Foo: 6


  return 0;
}