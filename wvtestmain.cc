#include "wvtest.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *prefix = "";
    if (argc > 1)
	prefix = argv[1];
    int ret = WvTest::run_all(prefix);
    
    // keep 'make' from aborting if this environment variable is set
    if (getenv("WVTEST_NO_FAIL"))
	return 0;
    else
	return ret;
}
