/** \file
 * A WvString example.
 */
/** \example wvstringex.cc
 * Some text about this example...
 */
#include "wvstring.h"
#include <stdio.h>
#include <assert.h>

int main()
{
    const char *mystring = "Cool!";

    // Creates x as a wrapper for mystring
    WvStringParm x(mystring);
    // ...x's internal string buffer points to mystring
    assert(x.cstr() == mystring);
    assert(strcmp(x, mystring) == 0);
    
    // Creates y as a copy of mystring
    WvString y(mystring);
    // ...y's internal string buffer points to a copy of mystring
    assert(y.cstr() != mystring);
    assert(strcmp(y, mystring) == 0);
    
    // Creates z as a copy of y
    WvString z(y);
    // ...z's internal string buffer points to y's
    assert(z.cstr() == y.cstr());
    // ...prove it by modifying y
    // (dangerous use of const_cast<>, see below for example of edit())
    const_cast<char*>(y.cstr())[0] = 'Z'; // change first char to Z
    assert(z.cstr()[0] == 'Z');
    // ...and make it point to a unique copy of the string
    z.unique(); // could also write z.edit()
    assert(z.cstr() != y.cstr());
    // ...prove it by modifying y again
    const_cast<char*>(y.cstr())[0] = 'Y'; // change first char to Y
    assert(z.cstr()[0] == 'Z'); // but z points to a different string

    // Notice that cstr() deliberately returns a const char* to make
    // it hard to accidentally modify an internal string buffer that
    // is shared by multiple WvStrings.  That is why the use of edit()
    // is preferred.  This automatically performs unique() then returns
    // a non-const pointer to the internal string buffer.
    // Consider:
    WvString w(z);
    // ...w's internal string buffer points to z's
    assert(w.cstr() == z.cstr());
    // ...but not anymore
    w.edit()[0] = 'W';
    assert(w.cstr() != z.cstr());
    assert(w.cstr()[0] == 'W');
    assert(z.cstr()[0] == 'Z');

    puts("Success!");
    return 0;
}
