/*
 * A WvStream example
 *
 * Some text about this example...
 */

#include <wvstream.h>

void mycallback(WvStream &s, void *userdata)
{
    WvStream *outstream = (WvStream *)userdata;
    
    char *str = s.getline(0);
    if (str)
	outstream->print("You said: %s\n", str);
}

int main()
{
    wvcon->setcallback(mycallback, wvcon);

    while (wvcon->isok())
    {
	if (wvcon->select(-1))
	    wvcon->callback();
    }
}
