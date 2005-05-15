#include "wvondiskhash.h"
#include "wvtest.h"
#include "wvxplc.h"
#include "wvstream.h"
#include "wvtest_filecountprefix.h"


typedef WvOnDiskHash<int, int> IntMap;

WVTEST_MAIN("dont create files in /tmp")
{
    WVPASS("start of test");
    
    // loads /tmp into cache to make things run consistantly fast.
    time_t s = time(NULL);
    system("ls /tmp |wc");
    printf("ls /tmp: elasped %d\n", time(NULL) - s);
    
    WVPASS("after reading /tmp");
    
    s = time(NULL);
    int before_count = wvtest_file_count_prefix("/tmp", "file");
    printf("before_count: elasped %d\n", time(NULL) - s);
    
    WVPASS("tick 2");

    s = time(NULL);
    int before_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    printf("before_count_var: elasped %d\n", time(NULL) - s);
    
    WVPASS("tick 3");

    IntMap *im = new IntMap;    
    WVPASS("tick 4");
    delete im;
    WVPASS("tick 5");

    s = time(NULL);
    int after_count = wvtest_file_count_prefix("/tmp", "file");
    printf("after_count: elasped %d\n", time(NULL) - s);
    
    WVPASS("tick 6");
    
    s = time(NULL);
    int after_count_var = wvtest_file_count_prefix("/var/tmp", "qdbm-annoy");
    printf("after_count_var: elasped %d\n", time(NULL) - s);
    
    WVPASSEQ(before_count, after_count);
    WVPASSEQ(before_count_var, after_count_var);

}

template<class Backend>
void itertest()
{
    WvOnDiskHash<int, int, Backend> *hash = new WvOnDiskHash<int, int, Backend>();
    WVPASS(hash->isok());

    //Add a bunch of elements
    int elements = 10;
    int data[elements];

    for (int i=0;i<elements;i++)
    {
        data[i] = i;
        hash->add(data[i], data[i]);
    }

    WVPASS(hash->isok());
    
    WvOnDiskHash<int, int, Backend>::Iter i(*hash);
    bool found[elements];
    memset(found,0,sizeof(found));

    bool passedIterate = true;
    //Iterate through, ensure that every data element is still there
    for (i.rewind();i.next();)
    {
        if (found[i()])
            passedIterate = false;
        found[i()] = true;
    }
    WVPASS(passedIterate); 

    bool passedExists = true;
    for (int j=0;j<elements;j++)
    {
        if (!hash->exists(j))
            passedExists = false;
    }
    WVPASS(passedExists);

    //Test removing elements
    hash->remove(1);
    hash->remove(2);
    
    WVPASS(hash->exists(0));
    WVFAIL(hash->exists(1));
    WVFAIL(hash->exists(2));

    WVPASS(hash->count() == elements-2);

    WVPASS((*hash)[0] == 0);
    WVPASS(hash->find(3) == 3);

    hash->zap();

    WVPASS(hash->isempty());

    delete hash;
}

#if 0
// FIXME: leaks and trips valgrind. Bug 7300.
WVTEST_MAIN("WvOnDiskHash (BdbHash backend)  with iteration and removal")
{
    itertest<WvBdbHash>();
}

WVTEST_MAIN("WvOnDiskHash (QdbmHash backend)  with iteration and removal")
{
    itertest<WvQdbmHash>();
}
#endif

