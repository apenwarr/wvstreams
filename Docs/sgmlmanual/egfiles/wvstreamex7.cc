/*
 * A WvStream example.
 *
 * Some text about this example...
 */

#include <wvstream.h>

int main()
{
    int nothing_count = 0;
    wvcon->autoforward(*wvcon);

    while (wvcon->isok())
    {
	if (wvcon->select(1000))
	{
	    nothing_count = 0;
	    wvcon->callback();
	}
	else
	{
	    nothing_count++;
	    wvcon->print("[TICK]");
	    if (nothing_count == 10)
	    {
		wvcon->print("[TIMEOUT]\n");
		break;
	    }
	}
    }
}
