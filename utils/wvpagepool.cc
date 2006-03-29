#include "wvpagepool.h"
#include "wvstream.h"
#include <errno.h>


#if 0
#define TRACE(format, args...) wverr->print(format ,##args)
#else
#define TRACE(format, args...)
#endif


WvPagePool::WvPagePool(WvStringParm filename, int flags, mode_t mode) :
    fd(-1),
    page_count(-1),
    pool(MAP_FAILED),
    first_free_page_id(-1)
{
    bool is_new = true;
    
    if (!!filename)
    {
        fd = ::open(filename, flags, mode);
        if (fd == -1)
        {
            seterr(errno);
            return;
        }
        
        struct stat st;
        if (::fstat(fd, &st) == -1)
        {
            seterr(errno);
            ::close(fd);
            fd = -1;
            return;
        }
        
        is_new = st.st_size == 0;
        
        page_count = (st.st_size + PAGE_SIZE - 1) / PAGE_SIZE;
        if (page_count < min_page_count)
            page_count = min_page_count;
        if (::ftruncate(fd, page_count<<PAGE_SHIFT) == -1)
        {
            seterr(errno);
            ::close(fd);
            fd = -1;
            return;
        }
    }
    else
    {
        page_count = min_page_count;
    }
    
    pool = ::mmap(NULL, page_count<<PAGE_SHIFT, mmap_prot(flags),
            mmap_flags(), fd, 0);
    if (pool == MAP_FAILED)
    {
        seterr(errno);
        if (fd != -1)
        {
            ::close(fd);
            fd = -1;
        }
        return;
    }
    else if (is_new)
        memset(pool, 0, PAGE_SIZE);
        
    for (first_free_page_id = 1;
            first_free_page_id < page_count;
            ++first_free_page_id)
        if (is_page_free(first_free_page_id))
            break;
}


WvPagePool::~WvPagePool()
{
    if (pool != MAP_FAILED)
    {
        ::munmap(pool, page_count<<PAGE_SHIFT);
    }
    if (fd != -1)
        ::close(fd);
}


int WvPagePool::resize(int new_page_count)
{
    TRACE("resize: before: page_count=%s\n", page_count);
    
    if (new_page_count < min_page_count)
        new_page_count = min_page_count;
    new_page_count = (new_page_count + min_page_count - 1)
            / min_page_count * min_page_count;
    
    if (new_page_count != page_count && pool != MAP_FAILED)
    {
        void *new_pool = mremap(pool, page_count<<PAGE_SHIFT,
                new_page_count<<PAGE_SHIFT, MREMAP_MAYMOVE);
        if (new_pool != MAP_FAILED)
        {
            if (fd != -1 && ::ftruncate(fd, new_page_count<<PAGE_SHIFT) == -1)
            {
                seterr(errno);
                ::munmap(new_pool, new_page_count<<PAGE_SHIFT);
                pool = MAP_FAILED;
                ::close(fd);
                fd = -1;
            }
            else
            {
                pool = new_pool;
                page_count = new_page_count;
            }
        }
    }
    
    TRACE("resize: after: page_count=%s\n", page_count);
    
    return page_count;
}


WvPagePool::page_id_t WvPagePool::alloc(int num_pages)
{
    TRACE("alloc: num_pages=%s\n", num_pages);

    if (!isok() || num_pages < 1)
        return null_page_id;
    
    TRACE("alloc: before: first_free_page_id=%s\n", first_free_page_id);

    int start_page_id = first_free_page_id, i;
    
restart:

    TRACE("alloc: restart: start_page_id=%s\n", start_page_id);
    
    start_page_id = find_next_free_page(start_page_id);
    
    if (start_page_id + num_pages > page_count)
    {
        if (resize(page_count + num_pages) < start_page_id + num_pages)
            return null_page_id;
    }
    
    for (i=0; i<num_pages; ++i)
    {
        if (!is_page_free(start_page_id+i))
        {
            start_page_id += i + 1;
            goto restart;
        }
    }
        
    for (i=0; i<num_pages; ++i)
        mark_page_usage(start_page_id+i, true);
    
    if (start_page_id == first_free_page_id)
        for (first_free_page_id = start_page_id + num_pages;
                first_free_page_id < page_count;
                ++first_free_page_id)
            if (is_page_free(first_free_page_id))
                break;
    
    TRACE("alloc: after: first_free_page_id=%s\n", first_free_page_id);
    
    debug_map();

    return start_page_id;
}


void WvPagePool::free(page_id_t page_id, int num_pages)
{
    TRACE("free: page_id=%s num_pages=%s\n", page_id, num_pages);

    if (!isok() || num_pages < 1)
        return;
        
    TRACE("free: before: first_free_page_id=%s\n", first_free_page_id);

    for (int i=0; i<num_pages; ++i)
    {
        assert(!is_page_free(page_id+i));
        mark_page_usage(page_id+i, false);
    }
    
    if (page_id < first_free_page_id)
        first_free_page_id = page_id;

    if (find_next_used_page(page_id) == page_count)
        resize(page_id);

    TRACE("free: after: first_free_page_id=%s\n", first_free_page_id);
    
    debug_map();
}


WvPagePool::page_id_t WvPagePool::find_next_free_page(page_id_t page_id) const
{
    TRACE("find_next_free_page: start: page_id=%s\n", page_id);
    
    while (page_id < page_count && !is_page_free(page_id))
    {
        if ((page_id & 0x7) == 0 && ((char *)pool)[page_id>>3] == ~(char)0)
            page_id += 8;
        else
            ++page_id;
    }
    
    TRACE("find_next_free_page: end: page_id=%s\n", page_id);

    return page_id;
}


WvPagePool::page_id_t WvPagePool::find_next_used_page(page_id_t page_id) const
{
    TRACE("find_next_used_page: start: page_id=%s\n", page_id);
    
    while (page_id < page_count && is_page_free(page_id))
    {
        if ((page_id & 0x7) == 0 && ((char *)pool)[page_id>>3] == (char)0)
            page_id += 8;
        else
            ++page_id;
    }
    
    TRACE("find_next_used_page: end: page_id=%s\n", page_id);

    return page_id;
}


void WvPagePool::debug_map() const
{
    TRACE("%s:", page_count);
    if (pool != MAP_FAILED)
    {
        for (int i=1; i<page_count; ++i)
            TRACE("%s", is_page_free(i)? ".": "X");
    }
    else
        TRACE("INVALID");
    TRACE("\n");
}
