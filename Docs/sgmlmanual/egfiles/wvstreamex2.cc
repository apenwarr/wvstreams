/*
 * A WvStream example.
 *
 * Some text about this example...
 */

#include <wvstream.h>

int main()
{
    while (wvcon->isok())
	wvcon->print("You said: %s\n", wvcon->getline(-1));
}
