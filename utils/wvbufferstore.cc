/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines basic buffer storage classes.
 * These are not intended for use directly by clients.
 * See "wvbufferbase.h" for the public API.
 */
#include "wvbufferstore.h"

struct MemOps
{
    inline void uninit_copy(void *target, const void *source,
        size_t count)
    {
        memcpy(target, source, count);
    }
    inline void copy(void *target, const void *source, size_t count)
    {
        memcpy(target, source, count);
    }
    inline void *newarray(size_t count)
    {
        return new unsigned char[count];
    }
    inline void deletearray(void *buf)
    {
        delete[] (unsigned char*)buf;
    }
} memops;

inline size_t roundup(size_t value, size_t boundary)
{
    size_t mod = value % boundary;
    return mod ? value + boundary - mod : value;
}


/***** WvBufferStore *****/

WvBufferStore::WvBufferStore(int _granularity) :
    granularity(_granularity)
{
}


void WvBufferStore::move(void *buf, size_t count)
{
    while (count > 0)
    {
        size_t amount = usedopt();
        assert(amount != 0 ||
            !"attempted to move() more than used()");
        if (amount > count)
            amount = count;
        const void *data = get(amount);
        memops.uninit_copy(buf, data, amount);
        ((unsigned char*)buf) += amount;
        count -= amount;
    }
}


void WvBufferStore::copy(void *buf, size_t count, int offset)
{
    while (count > 0)
    {
        size_t amount;
        const void *data = peek(offset, & amount, 1);
        assert(amount != 0 ||
            !"attempted to copy() with invalid offset");
        if (amount > count)
            amount = count;
        memops.uninit_copy(buf, data, amount);
        ((unsigned char*)buf) += amount;
        count -= amount;
        offset += amount;
    }
}


void WvBufferStore::put(const void *data, size_t count)
{
    while (count > 0)
    {
        size_t amount = freeopt();
        assert(amount != 0 ||
            !"attempted to put() more than free()");
        if (amount > count)
            amount = count;
        void *buf = alloc(amount);
        memops.uninit_copy(buf, data, amount);
        ((const unsigned char*)data) += amount;
        count -= amount;
    }
}


void WvBufferStore::fastput(const void *data, size_t count)
{
    void *buf = alloc(count);
    memops.uninit_copy(buf, data, count);
}


void WvBufferStore::poke(const void *data, size_t count, int offset)
{
    int limit = int(used());
    assert(offset <= limit ||
        !"attempted to poke() beyond end of buffer");
    int end = offset + count;
    if (end >= limit)
    {
        size_t tail = end - limit;
        count -= tail;
        put(((const unsigned char*)data) + count, tail);
    }
    while (count > 0)
    {
        size_t amount;
        void *buf = mutablepeek(offset, & amount, granularity);
        if (amount > count)
            amount = count;
        memops.copy(buf, data, amount);
        ((const unsigned char*)data) += amount;
        count -= amount;
    }
}


void WvBufferStore::basicmerge(WvBufferStore &instore, size_t count)
{
    // move bytes as efficiently as we can using only the public API
    if (count == 0)
        return;
    const void *indata = NULL;
    void *outdata = NULL;
    size_t inavail = 0;
    size_t outavail = 0;
    for (;;)
    {
        if (inavail == 0)
        {
            inavail = instore.usedopt();
            assert(inavail != 0 ||
                !"attempted to merge() more than instore.used()");
            if (inavail > count)
                inavail = count;
            indata = instore.get(inavail);
        }
        if (outavail == 0)
        {
            outavail = freeopt();
            assert(outavail != 0 ||
                !"attempted to merge() more than free()");
            if (outavail > count)
                outavail = count;
            outdata = alloc(outavail);
        }
        if (inavail < outavail)
        {
            memops.uninit_copy(outdata, indata, inavail);
            count -= inavail;
            outavail -= inavail;
            if (count == 0)
            {
                unalloc(outavail);
                return;
            }
            ((unsigned char*)outdata) += inavail;
            inavail = 0;
        }
        else
        {
            memops.uninit_copy(outdata, indata, outavail);
            count -= outavail;
            if (count == 0) return;
            inavail -= outavail;
            ((const unsigned char*)indata) += outavail;
            outavail = 0;
        }
    }
}



/***** WvInPlaceBufferStore *****/

WvInPlaceBufferStore::WvInPlaceBufferStore(int _granularity,
    void *_data, size_t _avail, size_t _size, bool _autofree) :
    WvBufferStore(_granularity), data(NULL)
{
    reset(_data, _avail, _size, _autofree);
}


WvInPlaceBufferStore::WvInPlaceBufferStore(int _granularity, size_t _size) :
    WvBufferStore(_granularity), data(NULL)
{
    reset(memops.newarray(_size), 0, _size, true);
}


WvInPlaceBufferStore::~WvInPlaceBufferStore()
{
    if (data && xautofree)
        memops.deletearray(data);
}


void WvInPlaceBufferStore::reset(void *_data, size_t _avail,
    size_t _size, bool _autofree = false)
{
    assert(_data != NULL || _avail == 0);
    if (data && _data != data && xautofree)
        memops.deletearray(data);
    data = _data;
    xautofree = _autofree;
    xsize = _size;
    setavail(_avail);
}


void WvInPlaceBufferStore::setavail(size_t _avail)
{
    assert(_avail <= xsize);
    readidx = 0;
    writeidx = _avail;
}


size_t WvInPlaceBufferStore::used() const
{
    return writeidx - readidx;
}


const void *WvInPlaceBufferStore::get(size_t count)
{
    assert(count <= writeidx - readidx ||
        !"attempted to get() more than used()");
    const void *tmpptr = ((const unsigned char*)data) + readidx;
    readidx += count;
    return tmpptr;
}


void WvInPlaceBufferStore::unget(size_t count)
{
    assert(count <= readidx ||
        !"attempted to unget() more than ungettable()");
    readidx -= count;
}


size_t WvInPlaceBufferStore::ungettable() const
{
    return readidx;
}


void WvInPlaceBufferStore::zap()
{
    readidx = writeidx = 0;
}


size_t WvInPlaceBufferStore::free() const
{
    return xsize - writeidx;
}


void *WvInPlaceBufferStore::alloc(size_t count)
{
    assert(count <= xsize - writeidx ||
        !"attempted to alloc() more than free()");
    void *tmpptr = ((unsigned char*)data) + writeidx;
    writeidx += count;
    return tmpptr;
}


void WvInPlaceBufferStore::unalloc(size_t count)
{
    assert(count <= writeidx - readidx ||
        !"attempted to unalloc() more than unallocable()");
    writeidx -= count;
}


size_t WvInPlaceBufferStore::unallocable() const
{
    return writeidx - readidx;
}


void *WvInPlaceBufferStore::mutablepeek(int offset, size_t *count,
    size_t mincount)
{
    if (offset < 0)
        assert(size_t(-offset) <= readidx ||
            !"attempted to peek() with invalid offset");
    else
        assert(size_t(offset) < writeidx - readidx ||
            !"attempted to peek() with invalid offset");
    if (count)
        *count = writeidx - readidx - offset;
    return ((unsigned char*)data) + readidx + offset;
}



/***** WvConstInPlaceBufferStore *****/

WvConstInPlaceBufferStore::WvConstInPlaceBufferStore(int _granularity,
    const void *_data, size_t _avail) :
    WvReadOnlyBufferStoreMixin<WvBufferStore>(_granularity), data(NULL)
{
    reset(_data, _avail);
}


void WvConstInPlaceBufferStore::reset(const void *_data, size_t _avail)
{
    assert(_data != NULL || _avail == 0);
    data = _data;
    setavail(_avail);
}


size_t WvConstInPlaceBufferStore::used() const
{
    return avail - readidx;
}


void WvConstInPlaceBufferStore::setavail(size_t _avail)
{
    avail = _avail;
    readidx = 0;
}


const void *WvConstInPlaceBufferStore::get(size_t count)
{
    assert(count <= avail - readidx ||
        !"attempted to get() more than used()");
    const void *ptr = ((const unsigned char*)data) + readidx;
    readidx += count;
    return ptr;
}


void WvConstInPlaceBufferStore::unget(size_t count)
{
    assert(count <= readidx ||
        !"attempted to unget() more than ungettable()");
    readidx -= count;
}


size_t WvConstInPlaceBufferStore::ungettable() const
{
    return readidx;
}


const void *WvConstInPlaceBufferStore::peek(int offset, size_t *count,
    size_t mincount)
{
    if (offset < 0)
        assert(size_t(-offset) <= readidx ||
            !"attempted to peek() with invalid offset");
    else
        assert(size_t(offset) < avail - readidx ||
            !"attempted to peek() with invalid offset");
    if (count)
        *count = avail - readidx - offset;
    return ((const unsigned char*)data) + readidx + offset;
}


void WvConstInPlaceBufferStore::zap()
{
    readidx = avail = 0;
}



/***** WvLinkedBufferStore *****/

WvLinkedBufferStore::WvLinkedBufferStore(int _granularity) :
    WvBufferStore(_granularity), totalused(0)
{
}


void WvLinkedBufferStore::append(WvBufferStore *buffer, bool autofree)
{
    list.append(buffer, autofree);
    totalused += buffer->used();
}


void WvLinkedBufferStore::prepend(WvBufferStore *buffer, bool autofree)
{
    list.prepend(buffer, autofree);
    totalused += buffer->used();
}


void WvLinkedBufferStore::unlink(WvBufferStore *buffer)
{
    WvBufferStoreList::Iter it(list);
    if (it.find(buffer))
    {
        totalused -= buffer->used();
        it.unlink(); // do not recycle the buffer
    }
}


size_t WvLinkedBufferStore::numbuffers() const
{
    return list.count();
}


size_t WvLinkedBufferStore::used() const
{
    return totalused;
}


size_t WvLinkedBufferStore::usedopt() const
{
    size_t count;
    WvBufferStoreList::Iter it(list);
    for (it.rewind(); it.next(); )
        if ((count = it->usedopt()) != 0)
            return count;
    return 0;
}


const void *WvLinkedBufferStore::get(size_t count)
{
    if (count == 0)
        return NULL;
    totalused -= count;
    // search for first non-empty buffer
    WvBufferStore *buf;
    size_t availused;
    for (;;)
    {
        assert(! list.isempty() ||
            !"attempted to get() more than used()");
        buf = list.first();
        availused = buf->used();
        if (availused != 0) break;
        // unlink the leading empty buffer
        WvBufferStoreList::Iter it(list);
        it.rewind(); it.next();
        do_xunlink(it);
    }
    // return data if we have enough
    if (availused >= count)
        return buf->get(count);
    // allocate a new buffer if there is not enough room to coalesce
    size_t availfree = buf->free();
    size_t needed = count - availused;
    if (availfree < needed)
    {
        buf = newbuffer(count);
        prepend(buf, true);
        needed = count;
    }
    // coalesce subsequent buffers into the first
    WvBufferStoreList::Iter it(list);
    it.rewind(); it.next();
    for (;;)
    {
        assert(it.next() || !"attempted to get() more than used()");
        WvBufferStore *itbuf = it.ptr();
        size_t chunk = itbuf->used();
        if (chunk > 0)
        {
            if (chunk > needed)
                chunk = needed;
            buf->merge(*itbuf, chunk);
            needed -= chunk;
            if (needed == 0) break;
        }
        do_xunlink(it); // buffer is now empty
    }
    return buf->get(count);
}


void WvLinkedBufferStore::unget(size_t count)
{
    if (count == 0)
        return;
    assert(! list.isempty() ||
        !"attempted to unget() more than ungettable()");
    totalused += count;
    list.first()->unget(count);
}


size_t WvLinkedBufferStore::ungettable() const
{
    if (list.isempty())
        return 0;
    return list.first()->ungettable();
}


void WvLinkedBufferStore::zap()
{
    totalused = 0;
    WvBufferStoreList::Iter it(list);
    for (it.rewind(); it.next(); )
        do_xunlink(it);
}


size_t WvLinkedBufferStore::free() const
{
    if (! list.isempty())
        return list.last()->free();
    return 0;
}


size_t WvLinkedBufferStore::freeopt() const
{
    if (! list.isempty())
        return list.last()->freeopt();
    return 0;
}


void *WvLinkedBufferStore::alloc(size_t count)
{
    if (count == 0)
        return NULL;
    assert(! list.isempty() ||
        !"attempted to alloc() more than free()");
    totalused += count;
    return list.last()->alloc(count);
}


void WvLinkedBufferStore::unalloc(size_t count)
{
    totalused -= count;
    while (count > 0)
    {
        assert(! list.isempty() ||
            !"attempted to unalloc() more than unallocable()");
        WvBufferStore *buf = list.last();
        size_t avail = buf->unallocable();
        if (count < avail)
        {
            buf->unalloc(count);
            break;
        }
        WvBufferStoreList::Iter it(list);
        it.find(buf);
        do_xunlink(it);
        count -= avail;
    }
}


size_t WvLinkedBufferStore::unallocable() const
{
    return totalused;
}


void *WvLinkedBufferStore::mutablepeek(int offset, size_t *count,
    size_t mincount)
{
    if (offset < 0)
    {
        assert(size_t(-offset) <= ungettable() ||
            !"attempted to peek() with invalid offset");
        return list.first()->mutablepeek(offset, count, mincount);
    }
    assert(size_t(offset) < used() ||
        !"attempted to peek() with invalid offset");
        
    // search for the buffer that contains the offset
    WvBufferStoreList::Iter it(list);
    it.rewind();
    WvBufferStore *buf;
    for (;;) {
        assert(it.next());
        buf = it.ptr();
        size_t len = buf->used();
        if (size_t(offset) < len)
            break;
        offset -= len;
    }
    return buf->mutablepeek(offset, count, mincount);
}


WvBufferStore *WvLinkedBufferStore::newbuffer(size_t minsize)
{
    minsize = roundup(minsize, granularity);
    return new WvInPlaceBufferStore(granularity, minsize);
        
}


void WvLinkedBufferStore::recyclebuffer(WvBufferStore *buffer)
{
    delete buffer;
}


void WvLinkedBufferStore::do_xunlink(WvBufferStoreList::Iter &it)
{
    WvBufferStore *buf = it.ptr();
    bool autofree = it.link->auto_free;
    it.link->auto_free = false;
    it.xunlink();
    if (autofree)
        recyclebuffer(buf);
}



/***** WvDynamicBufferStore *****/

WvDynamicBufferStore::WvDynamicBufferStore(size_t _granularity,
    size_t _minalloc, size_t _maxalloc) :
    WvLinkedBufferStore(_granularity),
    minalloc(_minalloc), maxalloc(_maxalloc)
{
    assert(maxalloc >= minalloc);
}


size_t WvDynamicBufferStore::free() const
{
    return UNLIMITED_FREE_SPACE;
}


size_t WvDynamicBufferStore::freeopt() const
{
    size_t avail = WvLinkedBufferStore::freeopt();
    if (avail == 0)
        avail = UNLIMITED_FREE_SPACE;
    return avail;
}


void *WvDynamicBufferStore::alloc(size_t count)
{
    if (count > WvLinkedBufferStore::free())
    {
        WvBufferStore *buf = newbuffer(count);
        append(buf, true);
    }
    return WvLinkedBufferStore::alloc(count);
}

WvBufferStore *WvDynamicBufferStore::newbuffer(size_t minsize)
{
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
    // try to find a suitable buffer in the pool
    WvBufferStoreList::Iter it(pool);
    for (it.rewind(); it.next(); )
    {
        WvBufferStore *buf = it.ptr();
        if (buf->free() >= minsize)
        {
            it.link->auto_free = false;
            it.unlink();
            return buf;
        }
    }
#endif

    // allocate a new buffer
    // try to approximate exponential growth by at least doubling
    // the amount of space available for immediate use
    size_t size = used();
    if (size < minsize * 2)
        size = minsize * 2;
    if (size < minalloc)
        size = minalloc;
    else if (size > maxalloc)
        size = maxalloc;
    if (size < minsize)
        size = minsize;
    return WvLinkedBufferStore::newbuffer(size);
}


#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
void WvDynamicBufferStore::recyclebuffer(WvBufferStore *buffer)
{
    // add the buffer to the pool
    buffer->zap();
    pool.append(buffer, true);
}
#endif



/***** WvEmptyBufferStore *****/

WvEmptyBufferStore::WvEmptyBufferStore(size_t _granularity) :
    WvWriteOnlyBufferStoreMixin<
        WvReadOnlyBufferStoreMixin<WvBufferStore> >(_granularity)
{
}



/***** WvBufferCursorStore *****/

WvBufferCursorStore::WvBufferCursorStore(size_t _granularity,
    WvBufferStore *_buf, int _start, size_t _length) :
    WvReadOnlyBufferStoreMixin<WvBufferStore>(_granularity),
    buf(_buf), start(_start), end(_start + _length), offset(0),
    tmpbuf(_granularity, 1024 * granularity, 1024 * granularity)
{
}


size_t WvBufferCursorStore::used() const
{
    int pos = getpos();
    return end - pos;
}


size_t WvBufferCursorStore::usedopt() const
{
    int pos = getpos();
    size_t avail;
    if (pos == 0)
        avail = buf->usedopt();
    else if (pos >= end)
        avail = 0;
    else
        buf->peek(pos, & avail, granularity);
    size_t maxavail = size_t(end - start);
    if (avail > maxavail)
        avail = maxavail;
    return avail;
}


const void *WvBufferCursorStore::get(size_t count)
{
    if (count == 0)
        return NULL;
    tmpbuf.zap();
    int pos = getpos();
    size_t avail;
    const void *ptr = buf->peek(pos, & avail, granularity);
    if (avail < count)
    {
        void *nptr = tmpbuf.alloc(count);
        buf->copy(nptr, count, pos);
        ptr = nptr;
    }
    offset += count;
    return ptr;
}


void WvBufferCursorStore::unget(size_t count)
{
    assert(count <= size_t(offset) ||
        !"attempted to unget() more than ungettable()");
    offset -= count;
}


size_t WvBufferCursorStore::ungettable() const
{
    return offset;
}


void WvBufferCursorStore::zap()
{
    start = end = offset = 0;
}


const void *WvBufferCursorStore::peek(int offset, size_t *count,
    size_t mincount)
{
    int pos = getpos();
    size_t avail;
    const void *ptr = buf->peek(pos, & avail, granularity);
    if (count)
    {
        size_t maxavail = size_t(end - start);
        if (avail > maxavail)
            avail = maxavail;
        *count = avail;
    }
    return ptr;
}


bool WvBufferCursorStore::iswritable() const
{
    // support mutablepeek() but nothing more
    return true;
}


void *WvBufferCursorStore::mutablepeek(int offset, size_t *count,
    size_t mincount)
{
    int pos = getpos();
    size_t avail;
    void *ptr = buf->mutablepeek(pos, & avail, granularity);
    if (count)
    {
        size_t maxavail = size_t(end - start);
        if (avail > maxavail)
            avail = maxavail;
        *count = avail;
    }
    return ptr;
}


int WvBufferCursorStore::getpos() const
{
    int pos = start + offset;
    assert(pos >= 0 && size_t(pos) <= buf->used() ||
        size_t(pos) >= buf->ungettable() ||
        !"attempted to operate on buffer cursor over invalid region");
    return pos;
}
