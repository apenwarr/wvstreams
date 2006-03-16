#include "wvtest.h"
#include "wvpagepool.h"
#include "wvfileutils.h"
#include "wvstream.h"
#include "wvfile.h"

static void dump_maps()
{
    WvFile f("/proc/self/maps", O_RDONLY);
    while (f.isok())
    {
        const char *line = f.blocking_getline(-1);
        if (line == NULL)
            break;
        wvout->print("%s\n", line);
    }
}


static void test_pagepool(WvPagePool &pp)
{
    int i;
    const int count = 16, max_num_pages = 64, iterations = 1024;
    WvPagePool::page_id_t page_id[count];
    int num_pages[count];

    WVPASS(pp.isok());
    
    for (i=0; i<count; ++i)
    {
        page_id[i] = 0;
        num_pages[i] = 0;
    }
    
    for (i=0; i<iterations; ++i)
    {
        int j = rand() % count;
        
        if (page_id[j] == 0)
        {
            num_pages[j] = rand() % max_num_pages + 1;
            wverr->print("%s: alloc %s\n", i, num_pages[j]);
            page_id[j] = pp.alloc(num_pages[j]);
            WVPASS(pp.addr(page_id[j]) != NULL);
            //dump_maps();
            memset(pp.addr(page_id[j]), rand()%256, pp.size(num_pages[j]));
        }
        else
        {
            wverr->print("%s: free %s, %s\n", i, page_id[j], num_pages[j]);
            memset(pp.addr(page_id[j]), rand()%256, pp.size(num_pages[j]));
            pp.free(page_id[j], num_pages[j]);
            //dump_maps();
            page_id[j] = 0;
        }
    }
    
    WVPASS(pp.isok());
}


WVTEST_MAIN("on-disk")
{
    WvString filename = wvtmpfilename("on-disk");
    WvPagePool pp(filename, O_RDWR | O_CREAT | O_TRUNC);
    
    test_pagepool(pp);
    
    unlink(filename);
}


WVTEST_MAIN("in-memory")
{
    WvPagePool pp(WvString::null, O_RDWR);
    
    test_pagepool(pp);
}


