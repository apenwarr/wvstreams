#include "wvtest.h"
#include "strutils.h"

WVTEST_MAIN("old-style")
{
    char *input[2] = {"abbababababbba", "abbababababbbablab"};
    char *desired[2] = {"abxaxxaxxaxxaxbbxax", "abxaxxaxxaxxaxbbxaxblab"};

    for (int i = 0; i < 2; i++)
    {
        WvString result = strreplace(input[i], "ba", "xax");
        if (!WVPASS(result == desired[i]))
            printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
    }
}
