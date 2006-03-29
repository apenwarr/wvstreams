#ifndef __WVPAGEPOOLSTRINGTABLE_H
#define __WVPAGEPOOLSTRINGTABLE_H

#include "wverror.h"
#include "wvpagepool.h"

class WvPagePoolStringTable : public WvError
{
public:

    typedef unsigned string_id_t;
    static const string_id_t null_string_id = ~0;
    
    WvPagePoolStringTable(WvPagePool *_pp, WvPagePool::page_id_t _header_page_id);
    ~WvPagePoolStringTable();
    
    string_id_t add(const char *str);
    string_id_t lookup(const char *str) const;
    const char *get(string_id_t string_id) const;
    int get_len(string_id_t string_id) const;
    int get_ref_count(string_id_t string_id) const;
    int remove(string_id_t string_id);
    
    WvPagePool::page_id_t get_header_page_id() const;
    
private:

    struct Header
    {
        WvPagePool::page_id_t start_page_id;
        int num_pages;
        int num_entries;
        int entry_size;
        int num_entries_used;
    };
    
    struct Entry
    {
        int ref_count;
        int type_and_len;
        bool in_use() const
            { return type_and_len != 0; }
        bool is_direct() const
            { return type_and_len & 0x80000000; }
        bool is_indirect() const
            { return !is_direct(); }
        int len() const
            { return in_use() && ref_count > 0? int(type_and_len & ~0x80000000): -1; }
        const char *str(WvPagePool *pp) const;
    };
    struct DirectEntry : public Entry
    {
        char str[1];
    };
    struct IndirectEntry : public Entry
    {
        WvPagePool::page_id_t start_page_id;
        int num_pages;
    };
    
    WvPagePool *pp;
    WvPagePool::page_id_t header_page_id;
    
    static const int num_pages_in_header = 1;
    int num_headers;
    int entry_index_bits, entry_index_mask;
    int header_index(string_id_t string_id) const
        { return int(unsigned(string_id)>>entry_index_bits); }
    int entry_index(string_id_t string_id) const
        { return int(unsigned(string_id)&entry_index_mask); }
    string_id_t make_string_id(int i, int j) const
        { return (i<<entry_index_bits) | j; }

    Header *header(int i) const
        { return &reinterpret_cast<Header *>(pp->addr(header_page_id))[i]; }
    Header *header_from_string_id(string_id_t string_id)
        { return header(header_index(string_id)); }
    
    Entry *entry(Header *h, int j) const
    {
        return reinterpret_cast<Entry *>(&((char *)(pp->addr(h->start_page_id)))[j*h->entry_size]);
    }
    Entry *entry(int i, int j) const
    {
        return entry(header(i), j);
    }
    Entry *entry_from_string_id(string_id_t string_id) const
    {
        return entry(header_index(string_id), entry_index(string_id));
    }
    
    Entry *find(const char *str, unsigned hash, int &i, int &j) const;

    void debug_dump(WvStringParm where) const;
    void debug_dump_header(WvStringParm where, int i) const;
    void debug_dump_entry(WvStringParm where, int i, int j) const;
};

#endif //__WVPAGEPOOLSTRINGTABLE_H
