#include "wvondiskhash.h"
#include "wvtest.h"
#include "wvxplc.h"
#include "wvstream.h"
#include "wvtest_filecountprefix.h"


typedef WvOnDiskHash<int, int> IntMap;

WVTEST_MAIN("dont create files in /tmp")
{
    int before_count = wvtest_file_count_prefix("/tmp", "file");
    int before_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");

    IntMap *im = new IntMap;    
    delete im;

    int after_count = wvtest_file_count_prefix("/tmp", "file");
    int after_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    WVPASS(before_count == after_count);
    WVPASS(before_count_var == after_count_var);

}

