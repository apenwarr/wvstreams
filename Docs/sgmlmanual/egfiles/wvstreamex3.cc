/** \file
 * A WvStream example.
 */
/** \example wvstreamex3.cc
 * Some text about this example...
 */

#include <wvstream.h>

int main()
{
    char buffer[10];
    size_t numread;
    
    while (wvcon->isok())
    {
	numread = wvcon->read(buffer, sizeof(buffer));
	if (numread) 
	{
	    wvcon->print("You said: ");
	    wvcon->write(buffer, numread);
	    wvcon->print(" (%s bytes)\n", numread);
	}
    }
}
