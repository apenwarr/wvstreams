#include "wvstring.h"
#include "wvlinklist.h"
   
DeclareWvList(WvString);   // creates class WvStringList
   
int main()
{
    WvStringList l;
    WvStringList::Iter i(l);
    WvString autostr("bork bork");
    
    l.append(new WvString("blah blah"), true); // auto-free enabled
    l.append(&autostr, false); // auto-free disabled: C++ will do this one
    // etc
    
    for (i.rewind(); i.next(); )
    {
	// we will learn a nicer way to do this with WvStream later.
        // we could typecast i() to (const char *), but the cstr() member
        // function is nicer (we all avoid typecasts when possible, right?)
	printf("%s\n", i().cstr());
    }
    
    // exiting this function will have C++ auto-free the list, which
    // causes the list to auto-free the "blah blah" string.  C++ also
    // auto-frees the "bork bork" string automatically.  It doesn't matter
    // that "bork bork" is freed before the list destructor is called; the
    // list doesn't refer to its members during destruction, unless it
    // needs to free the elements by itself.
}
