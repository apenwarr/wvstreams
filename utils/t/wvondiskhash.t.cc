#include "wvondiskhash.h"
#include "wvtest.h"
#include "wvxplc.h"
#include "wvstream.h"


typedef WvOnDiskHash<int, int> IntMap;

WVTEST_MAIN("dont create files in /tmp")
{
    // FIXME: The better way is to count the files that match.
    system("mkdir /tmp/saveme"); 
    system("mv /tmp/file* /tmp/saveme/");
    WVPASS(system("ls -l /tmp/file*") != 0 && "your test is broken");

    IntMap *im = new IntMap;    
    delete im;

    int f = system("ls -l /tmp/file*");
    printf("ls returned: %d\n", f);
    WVPASS(f != 0);

}

