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


template<class Backend>
void itertest_check(WvOnDiskHash<int, int, Backend> &hash, bool *seen,
		    size_t sizeof_seen, size_t &count)
{
    typename WvOnDiskHash<int, int, Backend>::Iter i(hash);

    memset(seen, 0, sizeof(seen));
    count = 0;

    for (i.rewind(); i.next(); )
    {
	int key = i.key();
	int value = *i;

	if (key == 1 && value == 11)
	{
	    WVFAIL(seen[0]);
	    seen[0] = true;
	}
	else if (key == 3 && value == 13)
	{
	    WVFAIL(seen[1]);
	    seen[1] = true;
	}
	else if (key == 7 && value == 17)
	{
	    WVFAIL(seen[2]);
	    seen[2] = true;
	}
	else
	{
	    WVFAIL("unknown key!");
	}
	++count;
    }
}


template<class Backend>
void itertest_new()
{
    WvOnDiskHash<int, int, Backend> hash;
    typename WvOnDiskHash<int, int, Backend>::Iter i(hash);
    bool seen[3];
    size_t count;

    WVPASS(hash.isok());

    hash.add(1, 11);
    hash.add(3, 13);
    hash.add(7, 17);

    WVPASS(hash.isok());
    WVPASS(hash.exists(1));
    WVPASS(hash.exists(3));
    WVPASS(hash.exists(7));
    WVFAIL(hash.exists(42));

    itertest_check(hash, seen, sizeof(seen), count);

    WVPASSEQ(count, 3);
    WVPASSEQ(hash.count(), 3);
    WVPASS(seen[0]);
    WVPASS(seen[1]);
    WVPASS(seen[2]);

    hash.remove(3);
    WVPASS(hash.exists(1));
    WVFAIL(hash.exists(3));
    WVPASS(hash.exists(7));

    itertest_check(hash, seen, sizeof(seen), count);

    WVPASSEQ(count, 2);
    WVPASSEQ(hash.count(), 2);
    WVPASS(seen[0]);
    WVFAIL(seen[1]);
    WVPASS(seen[2]);

    WVPASS(hash[1] == 11);
    WVPASS(hash.find(7) == 17);

    hash.zap();

    itertest_check(hash, seen, sizeof(seen), count);

    WVPASSEQ(count, 0);
    WVPASS(hash.isempty());
    WVPASSEQ(hash.count(), 0);
    WVFAIL(seen[0]);
    WVFAIL(seen[1]);
    WVFAIL(seen[2]);
}


#if 0
// FIXME: there's an evil leak, bug 7300
WVTEST_MAIN("WvOnDiskHash with WvQdbmHash backend")
{
    itertest_new<WvQdbmHash>();
}
#endif


WVTEST_MAIN("WvOnDiskHash with WvBdbHash backend")
{
    itertest_new<WvBdbHash>();
}

