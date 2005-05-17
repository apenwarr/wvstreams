#include "wvondiskhash.h"
#include "wvtest.h"
#include "wvxplc.h"
#include "wvstream.h"
#include "wvtest_filecountprefix.h"


WVTEST_MAIN("dont create files in /tmp")
{
    WVPASS("start of test");
    
    // loads /tmp into cache to make things run consistantly fast.
    time_t s = time(NULL);
    system("ls /tmp |wc");
    printf("ls /tmp: elasped %d\n", static_cast<int>(time(NULL) - s));
    
    WVPASS("after reading /tmp");
    
    s = time(NULL);
    int before_count = wvtest_file_count_prefix("/tmp", "file");
    printf("before_count: elasped %d\n", static_cast<int>(time(NULL) - s));
    
    WVPASS("tick 2");

    s = time(NULL);
    int before_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    printf("before_count_var: elasped %d\n", static_cast<int>(time(NULL) - s));
    
    WVPASS("tick 3");

    {
	WvOnDiskHash<int, int> im;
	WVPASS("tick 4");
    }
    WVPASS("tick 5");

    s = time(NULL);
    int after_count = wvtest_file_count_prefix("/tmp", "file");
    printf("after_count: elasped %d\n", static_cast<int>(time(NULL) - s));
    
    WVPASS("tick 6");
    
    s = time(NULL);
    int after_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    printf("after_count_var: elasped %d\n", static_cast<int>(time(NULL) - s));
    
    WVPASSEQ(before_count, after_count);
    WVPASSEQ(before_count_var, after_count_var);

}

