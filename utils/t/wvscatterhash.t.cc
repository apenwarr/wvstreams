
#include "wvtest.h"
#include "wvscatterhash.h"
#include "wvstring.h"


DeclareWvScatterTable2 (TestScatter, WvString);

WVTEST_MAIN("scatter hashing count, add, remove")
{
    const int size = 100; //must be larger the 50
    WvString *strings[size];
    for(int i=0;i<size;i++)
        strings[i] = new WvString(WvString("Test") + i);
    TestScatter scatterHash(size);
    for(unsigned int i=0;i<(unsigned)size;i++)
    {
        scatterHash.add(strings[i], true);
        WVPASS(scatterHash.count()==(i+1));
        WVPASS(scatterHash.slowcount()==(i+1));
    }
    
    for(int i=50;i<size;i++)
        scatterHash.remove(strings[i]);
    WVPASS(scatterHash.count()==50);
    scatterHash.zap();
    printf("scatter hash zaped");
    WVPASS(scatterHash.isempty());
    WVPASS(scatterHash.count()==0);
    
}

WVTEST_MAIN("scatter hashing isempty and autofree")
{
    const int size = 100;
    TestScatter scatterHash(size);
    WvString *test1 = new WvString("test1");
    WVPASS(scatterHash.isempty());
    scatterHash.add(test1, false);
    WVFAIL(scatterHash.isempty());
    WVFAIL(scatterHash.get_autofree(test1));
    scatterHash.set_autofree(test1, true);
    WVPASS(scatterHash.get_autofree(test1));
    scatterHash.zap();
    printf("scatter hash zaped");
    WVPASS(scatterHash.isempty());
    WVPASS(scatterHash.count()==0);
}


WVTEST_MAIN("scatter hashing Iter")
{
    const int size = 20;
    WvString *strings[size];
    for(int i=0;i<size;i++)
        strings[i] = new WvString(WvString("Test") + i);
    TestScatter scatterHash(size);
    for(int i=0;i<size;i++)
        scatterHash.add(strings[i], true);
    TestScatter::Iter i(scatterHash);
    for (i.rewind(); i.next(); )
    {
        printf("[%p]\n",i.ptr());
        bool isInThere = false;
        for(unsigned int count=0;count<scatterHash.count();count++)
        {
            if(i.ptr() == strings[count]) {
                isInThere = true;
                break;
            }
        }
        WVPASS(isInThere);
    }
    scatterHash.zap();
    printf("scatter hash zaped");
    WVPASS(scatterHash.isempty());
    WVPASS(scatterHash.count()==0);
}
