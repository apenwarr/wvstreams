#include <cstdlib>

#include "wvtest.h"
#include "wveditor.h"
#include "wvstring.h"

class LineEditor : public WvEditor<char>
{
    public:
    
    	LineEditor(const char *str)
    	    : WvEditor<char>(strlen(str), str)
    	{
    	}
};

bool operator ==(const WvEditor<char> &e, const char *str)
{
    size_t i;
    for (i=0; e[i] && str[i]; ++i)
    	if (e[i] != str[i])
    	    break;
    return e[i] == str[i];
}
	

WVTEST_MAIN("basic")
{
    LineEditor l1("This is a test."), l2("WORD");

    l1.insert(0, l2);
    WVPASS(l1 == "WORDThis is a test.");
    l1.insert(10, l2);
    WVPASS(l1 == "WORDThis iWORDs a test.");
    l1.insert(23, l2);
    WVPASS(l1 == "WORDThis iWORDs a test.WORD");
    l1.remove(7, 10);
    WVPASS(l1 == "WORDThi test.WORD");
    l1.insert(9, l2);
    WVPASS(l1 == "WORDThi tWORDest.WORD");
}

WVTEST_MAIN("reuse")
{
    LineEditor l1(""), l2("WORD");

    l1.insert(0, l2);
    WVPASS(l1 == "WORD");
    l1.insert(1, l2);
    WVPASS(l1 == "WWORDORD");
    l1.insert(2, l2);
    WVPASS(l1 == "WWWORDORDORD");
    l1.insert(11, l2);
    WVPASS(l1 == "WWWORDORDORWORDD");
    l1.insert(14, l2);
    WVPASS(l1 == "WWWORDORDORWORWORDDD");
    l1.remove(1, 8);
    WVPASS(l1 == "WORWORWORDDD");
    l1.remove(3, 8);
    WVPASS(l1 == "WORD");
    l1.remove(0, 4);
    WVPASS(l1 == "");
}
