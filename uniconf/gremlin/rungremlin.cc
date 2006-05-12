#include "unigremlin.h"
#include "wvstring.h"
#include <stdio.h>

#define TEST_GREMLIN 0

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	fprintf(stderr, "Usage: %s <uniconf-moniker> <start-key>\n",
		argv[0]);
	return 1;
    }
    
    WvString moniker = argv[1];
    WvString key = argv[2];
    if (!key)
        key = "/";

    UniConfGremlin g(moniker, key);
    printf("Gremlin created\n");
#if TEST_GREMLIN
        g.test();
#else
        g.start();
#endif
    printf("%s", g.status().cstr());
    return 0;
}
