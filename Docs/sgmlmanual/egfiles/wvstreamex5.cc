/** \file
 * A WvStream example.
 */
/** \example wvstreamex5.cc
 * Some text about this example...
 */

#include <wvstream.h>

int main()
{
    wvcon->autoforward(*wvcon);

    while (wvcon->isok())
    {
	if (wvcon->select(-1))
	    wvcon->callback();
    }
}
