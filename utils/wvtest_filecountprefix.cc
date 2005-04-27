#include "wvtest_filecountprefix.h"
#include "wvdiriter.h"
#include "wvtest.h"
#include <string.h>

#define SLIP_COUNT 500

int wvtest_file_count_prefix(WvStringParm dirname, WvStringParm prefix)
{
    WvDirIter di(dirname, false);
    int len = strlen(prefix);
    int count = 0;

    int i = 0;

    for (di.rewind(); di.next(); ) {
        if (strncmp(prefix, di->name, len) == 0) {
            count++;
        }
        if (++i % SLIP_COUNT == 0)
        {
            i = 0;
            WVPASS(".");
        }
    }
    return count;
}

