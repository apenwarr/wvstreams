#include "wvtest.h"

int main(int argc, char **argv)
{
    const char *prefix = "";
    if (argc > 1)
	prefix = argv[1];
    return WvTest::run_all(prefix);
}
