#ifndef __WVPAGEPOOL_H
#define __WVPAGEPOOL_H

#include "wverror.h"
#include <sys/mman.h>
#include <sys/user.h>
#include <assert.h>
#include <fcntl.h>

class WvPagePool : public WvError
{
public:

    typedef int page_id_t;
    static const page_id_t null_page_id = 0;
    
    WvPagePool(WvStringParm filename, int flags, mode_t mode = 0666);
    ~WvPagePool();

    size_t page_size() const
        { return PAGE_SIZE; }
    int page_shift() const
        { return PAGE_SHIFT; }

    void *addr(page_id_t page_id) const
    { 
        assert(page_id > 0 && page_id < page_count);
        return pool != MAP_FAILED? &((char *)pool)[page_id<<PAGE_SHIFT]: NULL;
    }
    size_t size(int num_pages) const
        { return size_t(num_pages) << PAGE_SHIFT; }

    page_id_t alloc(int num_pages);
    void free(page_id_t page_id, int num_pages);

private:

    int fd;
    int page_count;
    static const int min_page_count = 8;
    void *pool;
    page_id_t first_free_page_id;

    int resize(int new_page_count);
    
    bool is_page_free(page_id_t page_id) const
    {
        assert(page_id > 0 && page_id < page_count);
        return !(((char *)pool)[page_id>>3] & (1<<(page_id&0x7)));
    }
    void mark_page_usage(page_id_t page_id, bool used)
    {
        assert(page_id > 0 && page_id < page_count);
        if (used)
            ((char *)pool)[page_id>>3] |= (1<<(page_id&0x7));
        else
            ((char *)pool)[page_id>>3] &= ~(1<<(page_id&0x7));
    }
    
    page_id_t find_next_free_page(page_id_t page_id) const;
    page_id_t find_next_used_page(page_id_t page_id) const;

    static int mmap_prot(int open_flags)
    {
        switch (open_flags & O_ACCMODE)
        {
            case O_WRONLY: return PROT_WRITE;
            case O_RDONLY: return PROT_READ;
            default: return PROT_READ | PROT_WRITE;
        }
    }
    
    int mmap_flags() const
    {
        if (fd != -1)
            return MAP_SHARED;
        else
            return MAP_PRIVATE | MAP_ANONYMOUS;
    }    
    
    void debug_map() const;
};

#endif //__WVPAGEPOOL_H
