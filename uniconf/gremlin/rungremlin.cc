#include "unigremlin.h"
#include "wvstring.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
	fprintf(stderr, "Usage: %s <uniconf-moniker> <start-key>\n",
		argv[0]);
	return 1;
    }
    
    WvString moniker = argv[1];
    WvString key = argv[2];
    UniConfGremlin g(moniker, key, 5);
    printf("Gremlin created\n");
    g.start();
    printf("%s", g.status().cstr());
    return 0;
}
