#include "wvtest_filecountprefix.h"
#include "wvdiriter.h"
#include <string.h>

int wvtest_file_count_prefix(WvStringParm dirname, WvStringParm prefix)
{
    WvDirIter di(dirname, false);
    int len = strlen(prefix);
    int count = 0;

    for (di.rewind(); di.next(); ) {
        if (strncmp(prefix, di->name, len) == 0) {
            count++;
        }
    }
    return count;
}

