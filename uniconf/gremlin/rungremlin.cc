#include "unigremlin.h"
#include "wvstring.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    WvString moniker = argv[0];
    WvString key = argv[1];
    UniConfGremlin g(moniker, key, 5);
    printf("Gremlin created\n");
    g.start();
    printf("%s", g.status());
    return 0;
}
