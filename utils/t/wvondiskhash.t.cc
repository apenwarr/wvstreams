#include "wvondiskhash.h"
#include "wvtest.h"
#include "wvxplc.h"
#include "wvstream.h"
#include "wvtest_filecountprefix.h"


typedef WvOnDiskHash<int, int> IntMap;

WVTEST_MAIN("dont create files in /tmp")
{
    WVPASS("tick 1");
    int before_count = wvtest_file_count_prefix("/tmp", "file");
    WVPASS("tick 2");
    int before_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    WVPASS("tick 3");

    IntMap *im = new IntMap;    
    WVPASS("tick 4");
    delete im;
    WVPASS("tick 5");

    int after_count = wvtest_file_count_prefix("/tmp", "file");
    WVPASS("tick 6");
    int after_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    WVPASS(before_count == after_count);
    WVPASS(before_count_var == after_count_var);

}

