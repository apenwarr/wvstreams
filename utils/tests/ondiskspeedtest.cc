#include <stdint.h>
#include <time.h>
#include <wvondiskhash.h>
#include <wvlog.h>
#include <wvstring.h>
#include <wvtimeutils.h>

template <class HashType>
void SpeedDemon(WvStringParm name, int max)
{
    WvLog log("SpeedDemon");

    unlink("test.db");
    WvOnDiskHash<int, int, HashType> testhash("test.db");
    log("---------------------------------\n");
    log("Testing %s with %s records...\n", name, max);

    WvTime start = wvtime();
    for (int i = 0; i < max; ++i)
        testhash.add(i, i);

    log("    sequential write:    %s ms\n",
            msecdiff(wvtime(), start));
    start = wvtime();

    for (int i = 0; i < max; ++i)
    {
        int r = random() % max;
        testhash.add(r, r, true);
    }

    log("    random write:        %s ms\n",
            msecdiff(wvtime(), start));
    start = wvtime();

    for (int i = 0; i < max; ++i)
    {
        if (testhash.find(i) != i)
            log(WvLog::Error, "Error!!");
    }

    log("    sequential read:     %s ms\n",
            msecdiff(wvtime(), start));
    start = wvtime();

    for (int i = 0; i < max; ++i)
    {
        int r = random() % max;
        if (testhash.find(r) != r)
            log(WvLog::Error, "Error!!");
    }

    log("    random read:         %s ms\n",
            msecdiff(wvtime(), start));
    start = wvtime();

    typename WvOnDiskHash<int, int, HashType>::Iter i(testhash);
    for (i.rewind(); i.next(); ) {}

    log("    full iteration:      %s ms\n",
            msecdiff(wvtime(), start));
}

int main(int argc, char **argv)
{
    int numrecords = 25000;
    if (argc > 1 && atoi(argv[1]))
        numrecords = atoi(argv[1]);

    srandom(time(0));

#ifdef WITH_BDB
    SpeedDemon<WvBdbHash>("BDB hash", numrecords);
#else
    fprintf(stderr, "Not testing BDB hash!\n");
#endif

#ifdef WITH_QDBM
    SpeedDemon<WvQdbmHash>("QDBM hash", numrecords);
#else
    fprintf(stderr, "Not testing QDBM hash!\n");
#endif
}
