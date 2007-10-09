#include "wvcallbacklist.h"
#include "wvtest.h"
#include "wvtr1.h"


typedef wv::function<int(int&)> TestCallback;


/* The return value will be ignored. */
int testcb(int &count)
{
    ++count;
    return 42;
}


WVTEST_MAIN("WvCallbackList sanity")
{
    WvCallbackList<TestCallback> list;
    const unsigned int numcb = 42;
    char jar[numcb];
    int count;

    WVPASS(list.isempty());

    for (unsigned int i = 0; i < numcb; ++i)
	list.add(testcb, &jar[i]);

    WVPASS(!list.isempty());

    count = 0;
    list(count);

    WVPASSEQ(count, numcb);

    list.del(&jar[0]);

    count = 0;
    list(count);

    WVPASSEQ(count, numcb - 1);

    for (unsigned int i = 1; i < numcb; ++i)
	list.del(&jar[i]);

    WVPASS(list.isempty());

    count = 0;
    list(count);

    WVPASSEQ(count, 0);
}

