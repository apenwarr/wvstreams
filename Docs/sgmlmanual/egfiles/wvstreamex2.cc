#include <wvstream.h>

int main()
{
    while (wvcon->isok())
	wvcon->print("You said: %s\n", wvcon->getline(-1));
}
