#include "wvstream.h"

int main()
{
    wvout->print("copying stdin to stdout...\n");
    while (wvin->isok())
    {
	char *line = wvin->blocking_getline(-1);
	if (line)
	    wvout->print("%s\n", line);
	else
	    break;
    }
    return 0;
}
