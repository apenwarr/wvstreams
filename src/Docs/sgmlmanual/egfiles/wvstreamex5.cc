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
