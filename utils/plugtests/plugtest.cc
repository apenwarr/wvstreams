#include "wvxplc.h"
#include <xplc/xplc.h>
#include "wvmonikerregistry.h"
#include "iwvstream.h"
#include "wvstreamclone.h"
#include <stdio.h>

int main()
{
    XPLC xplc;
    xplc.addModuleDirectory(".");
    IWvStream *s = wvcreate<IWvStream>("test:footest");
    printf("monikerget: %p\n", s);
    WvStreamClone clone(s);
    char *line;
    while ((line = clone.getline(-1)) != NULL)
	printf("line: %s\n", line);
    
    return 0;
}
