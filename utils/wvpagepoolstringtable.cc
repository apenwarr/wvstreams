#include "wvpagepoolstringtable.h"
#include "wvstream.h"


#if 0
#define TRACE(format, args...) wverr->print(format ,##args)
#else
#define TRACE(format, args...)
#endif


WvPagePoolStringTable::WvPagePoolStringTable(WvPagePool *_pp,
        WvPagePool::page_id_t _header_page_id) :
    pp(_pp),
    header_page_id(_header_page_id),
    num_headers(pp->size(num_pages_in_header) / sizeof(Header))
{
    int header_index_bits;
    for (header_index_bits=1;
            (1<<(header_index_bits-1)) < num_headers;
            ++header_index_bits);
    entry_index_bits = 8*sizeof(WvPagePoolStringTable::string_id_t)
            - header_index_bits;
    entry_index_mask = (1<<entry_index_bits) - 1;

    if (header_page_id == WvPagePool::null_page_id)
    {
        header_page_id = pp->alloc(num_pages_in_header);
        if (header_page_id == WvPagePool::null_page_id)
        {
            seterr("Failed to allocate %s page(s): %s", pp->errstr());
            return;
        }
        TRACE("WvPagePoolStringTable: Allocated new header at %s\n",
                header_page_id); 
        for (int i=0; i<num_headers; ++i)
            memset(header(i), 0, sizeof(Header));
    }
    else
        TRACE("WvPagePoolStringTable: Using existing header at %s\n",
                header_page_id);
                
    debug_dump("WvPagePoolStringTable");
}


WvPagePoolStringTable::~WvPagePoolStringTable()
{
    debug_dump("~WvPagePoolStringTable");
}


WvPagePoolStringTable::Entry *WvPagePoolStringTable::find(const char *str,
        unsigned hash, int &i, int &j) const
{
    for (i=0; i<num_headers; ++i)
    {
        Header *h = header(i);
        if (h->start_page_id == WvPagePool::null_page_id)
            break;
     
        j = entry_index(hash % h->num_entries);
        int start_j = j;
        for (;;)
        {
            Entry *e = entry(h, j);
            if (!e->in_use())
                break;
                
            if (e->ref_count > 0 && strcmp(e->str(pp), str) == 0)
                return e;
                
            if (++j == h->num_entries)
                j = 0;
            if (j == start_j)
                break;
        }
    }

    return NULL;
}


WvPagePoolStringTable::string_id_t WvPagePoolStringTable::add(const char *str)
{
    string_id_t result;
    
    unsigned hash = WvHash(str);
    
    TRACE("add: str='%s' (hash=%s)\n", str, hash);
    
    // First, look to see if we already have the entry
    int i, j;
    Entry *e = find(str, hash, i, j);
    if (e)
    {
        TRACE("add: already at %s, %s; incrementing ref_count\n", i, j);
        ++e->ref_count;
        result = make_string_id(i, j);
    }
    else
    {
        TRACE("add: not found; finding hash table\n");
        
        Header *h;
        for (i=0; i<num_headers; ++i)
        {
            h = header(i);
            if (h->start_page_id == WvPagePool::null_page_id)
                break;
            if (h->num_entries_used < h->num_entries / 2)
                break;
        }
        if (i == num_headers)
        {
            TRACE("add: num_headers exhausted\n");
            seterr("num_headers exhausted");
            result = null_string_id;
        }
        else
        {
            if (h->start_page_id == WvPagePool::null_page_id)
            {
                TRACE("add: making new hash table #%s\n", i);
                if (i == 0)
                    h->num_pages = 4;
                else
                    h->num_pages = 4*header(i-1)->num_pages;
                
                h->entry_size = 64;
                h->num_entries = pp->size(h->num_pages) / h->entry_size;
                h->num_entries_used = 0;
                
                WvPagePool::page_id_t start_page_id = pp->alloc(h->num_pages);
                h = header(i);
                h->start_page_id = start_page_id;
                
                debug_dump_header("add: header", i);
            }
    
            TRACE("add: inserting into hash table #%s\n", i);
            Entry *e;
            int j = entry_index(hash % h->num_entries);
            for (;;)
            {
                e = entry(h, j);
                if (!e->in_use())
                    break;
                
                if (e->ref_count == 0)
                {
                    WvPagePool::page_id_t page_id_to_remove = WvPagePool::null_page_id;
                    int num_pages_to_remove;
                    if (e->is_indirect())
                    {
                        IndirectEntry *ie = static_cast<IndirectEntry *>(e);
                        page_id_to_remove = ie->start_page_id;
                        num_pages_to_remove = ie->num_pages;
                    }
                    e->type_and_len = 0;
                    if (page_id_to_remove != WvPagePool::null_page_id)
                    {
                        pp->free(page_id_to_remove, num_pages_to_remove);
                        e = entry(i, j);
                    }
                    
                    Header *h = header(i);
                    --h->num_entries_used;
                    
                    break;
                }
                        
                if (++j == h->num_entries)
                    j = 0;
            }
    
            TRACE("add: inserting at position #%s\n", j);
            int len = strlen(str);
            int max_direct_len = h->entry_size - sizeof(Entry) - 1;
            if (len <= max_direct_len)
            {
                TRACE("add: creating direct entry\n");
                
                DirectEntry *de = static_cast<DirectEntry *>(e);
                strcpy(de->str, str);
                
                de->ref_count = 1;
                de->type_and_len = len | 0x80000000;
            }
            else
            {
                TRACE("add: creating indirect entry\n");
                
                IndirectEntry *ie = static_cast<IndirectEntry *>(e);
                ie->num_pages = pp->num_pages_needed_for(len + 1);
                WvPagePool::page_id_t start_page_id = pp->alloc(ie->num_pages);
                ie = static_cast<IndirectEntry *>(entry(i, j));
                ie->start_page_id = start_page_id;
                char *s = reinterpret_cast<char *>(pp->addr(ie->start_page_id));
                strcpy(s, str);
        
                ie->ref_count = 1;
                ie->type_and_len = len & ~0x80000000;
            }
            
            h = header(i);
            ++h->num_entries_used;
            
            debug_dump_entry("add: entry", i, j);
            
            result = make_string_id(i, j);
        }
    }
    
    debug_dump("add");
    
    TRACE("add: returning %s\n", result);
    
    return result;
}


WvPagePoolStringTable::string_id_t WvPagePoolStringTable::lookup(const char *str) const
{
    int i, j;
    Entry *e = find(str, WvHash(str), i, j);
    if (e)
        return make_string_id(i, j);
    else
        return null_string_id;
}


const char *WvPagePoolStringTable::get(string_id_t string_id) const
{
    return entry_from_string_id(string_id)->str(pp);
}


int WvPagePoolStringTable::get_len(string_id_t string_id) const
{
    return entry_from_string_id(string_id)->len();
}


int WvPagePoolStringTable::get_ref_count(string_id_t string_id) const
{
    Entry *e = entry_from_string_id(string_id);
    return e->in_use()? e->ref_count: 0;
}


int WvPagePoolStringTable::remove(string_id_t string_id)
{
    Entry *e = entry_from_string_id(string_id);
    --e->ref_count;
    
    debug_dump("remove");
    
    return e->ref_count;
}


const char *WvPagePoolStringTable::Entry::str(WvPagePool *pp) const
{
    if (!in_use() || ref_count == 0)
    {
        return NULL;
    }
    else if (is_direct())
    {
        const DirectEntry *de = static_cast<const DirectEntry *>(this);
        return de->str;
    }
    else
    {
        const IndirectEntry *ie = static_cast<const IndirectEntry *>(this);
        return (const char *)pp->addr(ie->start_page_id);
    }
}


void WvPagePoolStringTable::debug_dump_entry(WvStringParm where, int i, int j) const
{
    const Entry *e = entry(i, j);
    if (e->in_use() == 0)
        return;
        
    TRACE("dump: %s: %s: %s: len=%s ref_count=%s",
            where, i, j, e->len(), e->ref_count);
    if (e->is_direct())
    {
        const DirectEntry *de = static_cast<const DirectEntry *>(e);
        TRACE(" direct str='%s'", de->str);
    }
    else
    {
        const IndirectEntry *ie = static_cast<const IndirectEntry *>(e);
        TRACE(" indirect start_page_id=%s num_pages=%s str='%s'",
                ie->start_page_id, ie->num_pages,
                (const char *)pp->addr(ie->start_page_id));
    }
    TRACE("\n");
}


void WvPagePoolStringTable::debug_dump_header(WvStringParm where, int i) const
{
    const Header *h = header(i);
    if (h->start_page_id == WvPagePool::null_page_id)
        return;
        
    TRACE("dump: %s: %s: start_page_id=%s num_pages=%s num_entries=%s"
            " entry_size=%s num_entries_used=%s\n",
            where, i, h->start_page_id, h->num_pages,
            h->num_entries, h->entry_size, h->num_entries_used);
    TRACE("dump: %s: %s: Start\n", where, i);
    for (int j=0; j<h->num_entries; ++j)
        debug_dump_entry(where, i, j); 
    TRACE("dump: %s: %s: End\n", where, i);
}


void WvPagePoolStringTable::debug_dump(WvStringParm where) const
{
    TRACE("dump: %s: Start\n", where);
    for (int i=0; i<num_headers; ++i)
        debug_dump_header(where, i);
    TRACE("dump: %s: End\n", where);
}


WvPagePool::page_id_t WvPagePoolStringTable::get_header_page_id() const
{
    return header_page_id;
}


