/* 
 * A WvStream example.
 *
 * Some documentation for WvStreams...
 */

#include <wvstream.h>

int main()
{
    while (wvin->isok())
	wvout->print("You said: %s\n", wvin->blocking_getline(-1));
}
