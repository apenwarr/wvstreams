#include "wvtest.h"
#include "wvstringmask.h"

#include <stdio.h>
#include <climits>

WVTEST_MAIN("wvstringmask")
{
    WvStringMask a, b(""), c(' '), d("cab");

    // null
    bool empty = true;
    for (int i = CHAR_MIN; i < 256; ++i)
    {
	if (a[i])
	{
	    printf("i == %d\n", i);
	    empty = false;
	}
    }
    WVPASS(empty);
    WVPASSEQ(a.first(), '\0');

    // empty string
    empty = true;
    for (int i = CHAR_MIN; i < 256; ++i)
    {
	if (b[i])
	{
	    printf("i == %d\n", i);
	    empty = false;
	}
    }
    WVPASS(empty);
    WVPASSEQ(b.first(), '\0');

    // space character
    empty = true;
    for (int i = CHAR_MIN; i < 256; ++i)
    {
	if (c[i])
	{
	    printf("i == %d\n", i);
	    empty = false;
	}
    }
    WVFAIL(empty);
    WVPASSEQ(c.first(), ' ');

    // string
    empty = true;
    for (int i = CHAR_MIN; i < 256; ++i)
    {
	switch (i)
	{
	case 'a':
	case 'b':
	case 'c':
	    if (!d[i])
	    {
		printf("i == %d\n", i);
		WVPASS(d[i]);
	    }
	    break;
	default:
	    if (d[i])
	    {
		printf("i == %d\n", i);
		WVFAIL(d[i]);
	    }
	}
    }
    WVPASSEQ(d.first(), 'c');
}
