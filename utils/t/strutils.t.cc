#include "wvtest.h"
#include "strutils.h"

WVTEST_MAIN("replacetest.cc")
{
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
}

WVTEST_MAIN("sizetoatest.cc")
{
    {
        long blocks = 987654321;
        long blocksize = 1000000;
        char *desired[15] = {"987.6 TB", "98.7 TB", "9.8 TB", "987.6 GB", 
            "98.7 GB", "9.8 GB", "987.6 MB", "98.7 MB", "9.8 MB", "987.6 KB",
            "98.7 KB", "9.8 KB", "987 bytes", "98 bytes", "9 bytes"};
        int i = 0;

        while(blocksize != 1)
        {
            if (!WVPASS(sizetoa(blocks, blocksize) == desired[i]))
                printf("   because [%s] != [%s]\n", 
                        sizetoa(blocks, blocksize).cstr(), desired[i]);
            blocksize /= 10;
            i++;
        }

        while(blocks)
        {
            if (!WVPASS(sizetoa(blocks, blocksize) == desired[i]))
                printf("   because [%s] != [%s]\n",
                        sizetoa(blocks, blocksize).cstr(), desired[i]);
            blocks /= 10;
            i++;
        }
    }
}
