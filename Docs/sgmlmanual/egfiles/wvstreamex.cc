/** \file 
 * A WvStream example.
 */

/*! \example wvstreamex.cc
 * Some documentation for WvStreams...
 */

#include <wvstream.h>

int main()
{
    while (wvin->isok())
	wvout->print("You said: %s\n", wvin->getline(-1));
}
