#include "strutils.h"
#include <stdio.h>

int main()
{
    puts(strreplace("abbababababbba", "ba", "xax"));
    puts(strreplace("abbababababbbablab", "ba", "xax"));
    return 0;
}
