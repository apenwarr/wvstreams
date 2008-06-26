/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvtclstring.h"
#include "wvstringlist.h"
#include "wvstringmask.h"

int main(int argc, char **argv)
{
    WvStringMask nasties(WVTCL_NASTY_SPACES);
    WvStringMask splitchars(WVTCL_SPLITCHARS);

    bool bad = false;
    
    // correct output (all on one line):
    //   'chicken hammer {} {banana split} {shameless{frog}parts}
    //   big\}monkey\ \{potatoes {hammer\}time} {"quotable quote"}'
    static const char *strarray[] = {
	    "chicken",
	    "hammer",
	    "",
	    "banana split",
            "split\nends",
	    "shameless{frog}parts",
	    "big}monkey {potatoes",
	    "hammer\\}time",
	    "\"quotable quote\"",
	    NULL
    };
    WvStringList l;
    l.fill(strarray);
    
    WvString ls(wvtcl_encode(l, nasties, splitchars));
    printf("     List: '%s'\n", ls.cstr());
    printf("Unescaped: '%s'\n", wvtcl_unescape(ls).cstr());
    
    // wvtcl_encode (like the real tcl list encoder) will never put things
    // in quotes by itself.  However, the list decoder needs to be able to
    // handle them, in case the user provides them by hand.  So we'll just
    // tack some quoted strings onto the end.
    // 
    // We also take this opportunity to test handling of whitespace.
    // 
    ls.append("    \n  \t  \"unquotable quote\"\r\n");
    ls.append("  \"{embraced\\nunquotable}\"  \"quote\"whacker");
    
    // add the expected new strings to the end of the main list, for comparison
    // purposes below.
    l.append(new WvString("unquotable quote"), true);
    l.append(new WvString("{embraced\nunquotable}"), true);
    l.append(new WvString("quote"), true);
    l.append(new WvString("whacker"), true);
    
    printf("\nList split results:\n");
    WvStringList l2;
    wvtcl_decode(l2, ls, splitchars);
    
    WvStringList::Iter i(l), i2(l2);
    for (i.rewind(), i2.rewind(); i.next(), i2.next(), true; )
    {
	if (!i.cur() && !i2.cur())
	    break;
	
	if (!i.cur())
	{
	    printf("Extra element in list 2: '%s'\n", i2->cstr());
	    bad = true;
	    break;
	}
	
	if (!i2.cur())
	{
	    printf("Extra element in list 1: '%s'\n", i->cstr());
	    bad = true;
	    break;
	}
	
	printf("   '%s'\n", i2->cstr());
	
	if (*i != *i2)
	{
	    printf("     --> should be '%s'\n", i->cstr());
	    bad = true;
	}
    }
    
    if (bad)
	printf("Lists don't match!\n");
    else
	printf("Lists match.\n");
    return bad;
}
