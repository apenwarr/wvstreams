#include "wvdiriter.h"

int main()
{
    int guard1 = 0;
    int guard2 = 0;
    int guard3 = 0;
    int guard4 = 0;
    printf("Before: %d %d %d %d\n", guard1, guard2, guard3, guard4);
    WvDirIter i("/");
    WvDirIter *i2 = new WvDirIter("/");
    printf("After: %d %d %d %d\n", guard1, guard2, guard3, guard4);
}
