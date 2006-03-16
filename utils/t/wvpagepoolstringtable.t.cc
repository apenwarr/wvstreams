#include "wvtest.h"
#include "wvpagepoolstringtable.h"
#include "wvstream.h"
#include "wvfileutils.h"

WVTEST_MAIN("main")
{
    WvString filename = wvtmpfilename("on-disk");
    WvPagePool pp(filename, O_RDWR);
    WvPagePoolStringTable st(&pp, WvPagePool::null_page_id);
    
    WvString short_str = "This is a test";
    WvString long_str = "This is a test of a long string which means that it has lots of characters in it";
    
    WvPagePoolStringTable::string_id_t string_id, long_string_id;
    
    WVPASS(st.isok());
    WVPASS(st.get_header_page_id() != WvPagePool::null_page_id);
    string_id = st.add(short_str);
    WVFAILEQ(string_id, WvPagePoolStringTable::null_string_id);
    WVPASSEQ(st.get(string_id), short_str);
    WVPASSEQ(st.get_len(string_id), short_str.len());
    WVPASSEQ(st.get_ref_count(string_id), 1);
    WVPASSEQ(st.add(short_str), string_id);
    WVPASSEQ(st.get_ref_count(string_id), 2);
    WVPASSEQ(st.remove(string_id), 1);
    WVPASSEQ(st.get_ref_count(string_id), 1);
    
    long_string_id = st.add(long_str);
    WVFAILEQ(long_string_id, WvPagePoolStringTable::null_string_id);
    WVPASSEQ(st.get_len(long_string_id), long_str.len());
    WVPASSEQ(st.remove(long_string_id), 0);
    
    const int count = 256;
    WvString strs[count];
    WvPagePoolStringTable::string_id_t string_ids[count];
    for (int i=0; i<count; ++i)
    {
        if (i % 4 == 0)
            strs[i] = WvString("This is string super long super long This as string string asdas asdf #%s", i);
        else
            strs[i] = WvString("This is string This aslasdfjknasdjkf alsdnfalksn dflan sdflna sldfkna sldnfkl string asdas asdf #%s", i);
    }
    for (int i=0; i<count; ++i)
    {
        string_ids[i] = st.add(strs[i]);
        WVFAILEQ(string_ids[i], WvPagePoolStringTable::null_string_id);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
    }
    for (int i=17; ; i=(i+17)%count)
    {
        WVPASSEQ(st.lookup(strs[i]), string_ids[i]);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
        WVPASSEQ(st.add(strs[i]), string_ids[i]);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 2);
        if (i==0) break;
    }
    for (int i=29; ; i=(i+29)%count)
    {
        WVPASSEQ(st.lookup(strs[i]), string_ids[i]);
        WVPASSEQ(st.remove(string_ids[i]), 1);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
        WVPASSEQ(st.remove(string_ids[i]), 0);
        WVPASSEQ(st.get(string_ids[i]), NULL);
        WVPASSEQ(st.get_len(string_ids[i]), -1);
        WVPASSEQ(st.get_ref_count(string_ids[i]), 0);
        if (i==0) break;
    }
    for (int i=31; ; i=(i+31)%count)
    {
        string_ids[i] = st.add(strs[i]);
        WVFAILEQ(string_ids[i], WvPagePoolStringTable::null_string_id);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
        if (i==0) break;
    }
    for (int i=17; ; i=(i+17)%count)
    {
        WVPASSEQ(st.lookup(strs[i]), string_ids[i]);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
        WVPASSEQ(st.add(strs[i]), string_ids[i]);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 2);
        if (i==0) break;
    }
    for (int i=29; ; i=(i+29)%count)
    {
        WVPASSEQ(st.lookup(strs[i]), string_ids[i]);
        WVPASSEQ(st.remove(string_ids[i]), 1);
        WVPASSEQ(st.get(string_ids[i]), strs[i]);
        WVPASSEQ(st.get_len(string_ids[i]), strs[i].len());
        WVPASSEQ(st.get_ref_count(string_ids[i]), 1);
        WVPASSEQ(st.remove(string_ids[i]), 0);
        WVPASSEQ(st.get(string_ids[i]), NULL);
        WVPASSEQ(st.get_len(string_ids[i]), -1);
        WVPASSEQ(st.get_ref_count(string_ids[i]), 0);
        if (i==0) break;
    }
    
    unlink(filename);
}


