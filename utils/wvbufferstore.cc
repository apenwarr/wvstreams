/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines basic buffer storage classes.
 * These are not intended for use directly by clients.
 * See "wvbufferbase.h" for the public API.
 */
#include "wvbufferstore.h"
#include <string.h>

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
    inline void uninit_move(void *target, const void *source,
        size_t count)
    {
        memmove(target, source, count);
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

/**
 * Rounds the value up to the specified boundary.
 */
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


size_t WvBufferStore::peekable(int offset) const
{
    if (offset == 0)
    {
        return used();
    }
    else if (offset < 0)
    {
        if (size_t(-offset) <= ungettable())
            return size_t(-offset) + used();
    }
    else
    {
        int avail = int(used()) - offset;
        if (avail > 0)
            return avail;
    }
    return 0; // out-of-bounds
}


void WvBufferStore::move(void *buf, size_t count)
{
    while (count > 0)
    {
        size_t amount = optgettable();
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


void WvBufferStore::copy(void *buf, int offset, size_t count)
{
    while (count > 0)
    {
        size_t amount = optpeekable(offset);
        assert(amount != 0 ||
            !"attempted to copy() with invalid offset");
        if (amount > count)
            amount = count;
        const void *data = peek(offset, amount);
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
        size_t amount = optallocable();
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


void WvBufferStore::poke(const void *data, int offset, size_t count)
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
        size_t amount = optpeekable(offset);
        assert(amount != 0 ||
            !"attempted to poke() with invalid offset");
        if (amount > count)
            amount = count;
        void *buf = mutablepeek(offset, amount);
        memops.copy(buf, data, amount);
        ((const unsigned char*)data) += amount;
        count -= amount;
        offset += amount;
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
            inavail = instore.optgettable();
            assert(inavail != 0 ||
                !"attempted to merge() more than instore.used()");
            if (inavail > count)
                inavail = count;
            indata = instore.get(inavail);
        }
        if (outavail == 0)
        {
            outavail = optallocable();
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


void *WvInPlaceBufferStore::mutablepeek(int offset, size_t count)
{
    if (count == 0)
        return NULL;
    assert(((offset <= 0) ? 
        size_t(-offset) <= readidx :
        size_t(offset) < writeidx - readidx) ||
        ! "attempted to peek() with invalid offset or count");
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
        ! "attempted to get() more than used()");
    const void *ptr = ((const unsigned char*)data) + readidx;
    readidx += count;
    return ptr;
}


void WvConstInPlaceBufferStore::unget(size_t count)
{
    assert(count <= readidx ||
        ! "attempted to unget() more than ungettable()");
    readidx -= count;
}


size_t WvConstInPlaceBufferStore::ungettable() const
{
    return readidx;
}


const void *WvConstInPlaceBufferStore::peek(int offset, size_t count)
{
    if (count == 0)
        return NULL;
    assert(((offset <= 0) ? 
        size_t(-offset) <= readidx :
        size_t(offset) < avail - readidx) ||
        ! "attempted to peek() with invalid offset or count");
    return ((const unsigned char*)data) + readidx + offset;
}


void WvConstInPlaceBufferStore::zap()
{
    readidx = avail = 0;
}



/***** WvCircularBufferStore *****/

WvCircularBufferStore::WvCircularBufferStore(int _granularity,
    void *_data, size_t _avail, size_t _size, bool _autofree) :
    WvBufferStore(_granularity), data(NULL)
{
    reset(_data, _avail, _size, _autofree);
}


WvCircularBufferStore::WvCircularBufferStore(int _granularity, size_t _size) :
    WvBufferStore(_granularity), data(NULL)
{
    reset(memops.newarray(_size), 0, _size, true);
}


WvCircularBufferStore::~WvCircularBufferStore()
{
    if (data && xautofree)
        memops.deletearray(data);
}


void WvCircularBufferStore::reset(void *_data, size_t _avail,
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


void WvCircularBufferStore::setavail(size_t _avail)
{
    assert(_avail <= xsize);
    head = 0;
    totalused = totalinit = _avail;
}


size_t WvCircularBufferStore::used() const
{
    return totalused;
}


size_t WvCircularBufferStore::optgettable() const
{
    size_t avail = xsize - head;
    if (avail > totalused)
        avail = totalused;
    return avail;
}


const void *WvCircularBufferStore::get(size_t count)
{
    assert(count <= totalused ||
        ! "attempted to get() more than used()");
    size_t first = ensurecontiguous(0, count, false /*keephistory*/);
    const void *tmpptr = ((const unsigned char*)data) + first;
    head = (head + count) % xsize;
    totalused -= count;
    return tmpptr;
}


void WvCircularBufferStore::unget(size_t count)
{
    assert(count <= totalinit - totalused ||
        !"attempted to unget() more than ungettable()");
    head = (head + xsize - count) % xsize;
    totalused += count;
}


size_t WvCircularBufferStore::ungettable() const
{
    return totalinit - totalused;
}


void WvCircularBufferStore::zap()
{
    head = 0;
    totalused = totalinit = 0;
}


size_t WvCircularBufferStore::free() const
{
    return xsize - totalused;
}


size_t WvCircularBufferStore::optallocable() const
{
    size_t tail = head + totalused;
    if (tail >= xsize)
        return xsize - totalused;
    return xsize - tail;
}


void *WvCircularBufferStore::alloc(size_t count)
{
    assert(count <= xsize - totalused ||
        !"attempted to alloc() more than free()");
    totalinit = totalused; // always discard history
    size_t first = ensurecontiguous(totalused, count,
        false /*keephistory*/);
    void *tmpptr = ((unsigned char*)data) + first;
    totalused += count;
    totalinit += count;
    return tmpptr;
}


void WvCircularBufferStore::unalloc(size_t count)
{
    assert(count <= totalused ||
        !"attempted to unalloc() more than unallocable()");
    totalused -= count;
    totalinit -= count;
}


size_t WvCircularBufferStore::unallocable() const
{
    return totalused;
}


void *WvCircularBufferStore::mutablepeek(int offset, size_t count)
{
    if (count == 0)
        return NULL;
    assert(((offset <= 0) ? 
        size_t(-offset) <= totalinit - totalused :
        size_t(offset) < totalused) ||
        ! "attempted to peek() with invalid offset or count");
    size_t first = ensurecontiguous(offset, count,
        true /*keephistory*/);
    void *tmpptr = ((unsigned char*)data) + first;
    return tmpptr;
}


void WvCircularBufferStore::normalize()
{
    // discard history to minimize data transfers
    totalinit = totalused;

    // normalize the buffer
    compact(data, xsize, head, totalused);
    head = 0;
}


size_t WvCircularBufferStore::ensurecontiguous(int offset,
    size_t count, bool keephistory)
{
    // determine the region of interest
    size_t start = (head + offset + xsize) % xsize;
    if (count != 0)
    {   
        size_t end = start + count;
        if (end > xsize)
        {
            // the region is not entirely contiguous
            // determine the region that must be normalized
            size_t keepstart = head;
            if (keephistory)
            {
                // adjust the region to include history
                keepstart += totalused - totalinit + xsize;
            }
            else
            {
                // discard history to minimize data transfers
                totalinit = totalused;
            }
            keepstart %= xsize;

            // normalize the buffer over this region
            compact(data, xsize, keepstart, totalinit);
            head = totalinit - totalused;

            // compute the new start offset
            start = (head + offset + xsize) % xsize;
        }
    }
    return start;
}


void WvCircularBufferStore::compact(void *data, size_t size,
    size_t head, size_t count)
{
    // Case 1: Empty region
    if (count == 0)
        return;

    // Case 2: Contiguous region
    if (head + count <= size)
    {
        memops.uninit_move(data,
            ((unsigned char*)data) + head, count);
        return;
    }
    
    // Case 3: Non-contiguous region
    // FIXME: this is an interim solution
    // We can actually do this whole thing in place with a little
    // more effort...
    size_t headcount = size - head;
    size_t tailcount = count - headcount;
    void *buf = memops.newarray(tailcount);
    memops.uninit_move(buf, data, tailcount);
    memops.uninit_move(data,
        ((unsigned char*)data) + head, headcount);
    memops.uninit_move(((unsigned char*)data) + headcount,
        buf, tailcount);
    memops.deletearray(buf);
}



/***** WvLinkedBufferStore *****/

WvLinkedBufferStore::WvLinkedBufferStore(int _granularity) :
    WvBufferStore(_granularity), totalused(0), maxungettable(0)
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
    maxungettable = 0;
}


void WvLinkedBufferStore::unlink(WvBufferStore *buffer)
{
    WvBufferStoreList::Iter it(list);
    if (it.find(buffer))
    {
        totalused -= buffer->used();
        if (buffer == list.first())
            maxungettable = 0;
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


size_t WvLinkedBufferStore::optgettable() const
{
    size_t count;
    WvBufferStoreList::Iter it(list);
    for (it.rewind(); it.next(); )
        if ((count = it->optgettable()) != 0)
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
    WvBufferStoreList::Iter it(list);
    for (;;)
    {
        it.rewind(); it.next();
        buf = it.ptr();
        assert(buf ||
            !"attempted to get() more than used()");
        availused = buf->used();
        if (availused != 0)
            break;
        // unlink the leading empty buffer
        do_xunlink(it);
        maxungettable = 0;
    }

    // return the data
    if (availused < count)
        buf = coalesce(it, count);
    maxungettable += count;
    return buf->get(count);
}


void WvLinkedBufferStore::unget(size_t count)
{
    if (count == 0)
        return;
    assert(! list.isempty() || count > maxungettable ||
        !"attempted to unget() more than ungettable()");
    totalused += count;
    maxungettable -= count;
    list.first()->unget(count);
}


size_t WvLinkedBufferStore::ungettable() const
{
    if (list.isempty())
        return 0;
    size_t avail = list.first()->ungettable();
    if (avail > maxungettable)
        avail = maxungettable;
    return avail;
}


void WvLinkedBufferStore::zap()
{
    totalused = 0;
    maxungettable = 0;
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


size_t WvLinkedBufferStore::optallocable() const
{
    if (! list.isempty())
        return list.last()->optallocable();
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


size_t WvLinkedBufferStore::optpeekable(int offset) const
{
    // search for the buffer that contains the offset
    WvBufferStoreList::Iter it(list);
    offset = search(it, offset);
    WvBufferStore *buf = it.ptr();
    if (! buf)
        return 0; // out of bounds
    return buf->optpeekable(offset);
}


void *WvLinkedBufferStore::mutablepeek(int offset, size_t count)
{
    if (count == 0)
        return NULL;
    
    // search for the buffer that contains the offset
    WvBufferStoreList::Iter it(list);
    offset = search(it, offset);
    WvBufferStore *buf = it.ptr();
    assert(buf ||
        ! "attempted to peek() with invalid offset or count");
    
    // return data if we have enough
    size_t availpeek = buf->peekable(offset);
    if (availpeek < count)
    {
        // if this is the first buffer, then we need to unget as
        // much as possible to ensure it does not get discarded
        // during the coalescing phase
        size_t mustskip = 0;
        if (buf == list.first())
        {
            mustskip = ungettable();
            buf->unget(mustskip);
        }
        buf = coalesce(it, count);
        if (mustskip != 0)
            buf->skip(mustskip);
    }
    return buf->mutablepeek(offset, count);
}


void WvLinkedBufferStore::merge(WvBufferStore &instore, size_t count)
{
    if (count == 0)
        return;

    // determine if we can just do a fast merge
    WvLinkedBufferStore *inlinked =
        wvdynamic_cast<WvLinkedBufferStore*>(& instore);
    if (! inlinked)
    {
        basicmerge(instore, count);
        return;
    }
    WvLinkedBufferStore &other = *inlinked;

    // merge quickly by simply stealing internal buffers away from
    // the other buffer
    other.maxungettable = 0;
    WvBufferStoreList::Iter it(other.list);
    for (it.rewind(); it.next(); )
    {
        WvBufferStore *buf = it.ptr();
        size_t avail = buf->used();
        if (avail > count)
        {
            basicmerge(*buf, count);
            return; // done!
        }
        // move the entire buffer
        bool autofree = it.link->auto_free;
        it.link->auto_free = false;
        it.xunlink();
        other.totalused -= avail;
        append(buf, autofree);

        count -= avail;
        if (count == 0)
            return;
    }
    assert(count == 0 ||
        ! "attempted to merge() more than other.used()");
}


WvBufferStore *WvLinkedBufferStore::newbuffer(size_t minsize)
{
    minsize = roundup(minsize, granularity);
    //return new WvInPlaceBufferStore(granularity, minsize);
    return new WvCircularBufferStore(granularity, minsize);
}


void WvLinkedBufferStore::recyclebuffer(WvBufferStore *buffer)
{
    delete buffer;
}


int WvLinkedBufferStore::search(WvBufferStoreList::Iter &it,
    int offset) const
{
    it.rewind();
    if (it.next())
    {
        if (offset < 0)
        {
            // inside unget() region
            WvBufferStore *buf = it.ptr();
            if (size_t(-offset) <= buf->ungettable())
                return offset;
            it.rewind(); // mark out of bounds
        }
        else
        {
            // inside get() region
            do
            {
                WvBufferStore *buf = it.ptr();
                size_t avail = buf->used();
                if (size_t(offset) < avail)
                    return offset;
                offset -= avail;
            }
            while (it.next());
        }
    }
    return 0;
}


WvBufferStore *WvLinkedBufferStore::coalesce(
    WvBufferStoreList::Iter &it, size_t count)
{
    WvBufferStore *buf = it.ptr();
    size_t availused = buf->used();
    if (count <= availused)
        return buf;

    // allocate a new buffer if there is not enough room to coalesce
    size_t needed = count - availused;
    size_t availfree = buf->free();
    if (availfree < needed)
    {
        needed = count;
        buf = newbuffer(needed);
        
        // insert the buffer before the previous link
        list.add_after(it.prev, buf, true);
        it.find(buf);
    }
    
    // coalesce subsequent buffers into the first
    while (it.next())
    {
        WvBufferStore *itbuf = it.ptr();
        size_t chunk = itbuf->used();
        if (chunk > 0)
        {
            if (chunk > needed)
                chunk = needed;
            buf->merge(*itbuf, chunk);
            needed -= chunk;
            if (needed == 0)
                return buf;
        }
        do_xunlink(it); // buffer is now empty
    }
    assert(! "invalid count during get() or peek()");
    return NULL;
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


size_t WvDynamicBufferStore::optallocable() const
{
    size_t avail = WvLinkedBufferStore::optallocable();
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
    buf(_buf), start(_start), length(_length), shift(0)
{
}


bool WvBufferCursorStore::isreadable() const
{
    return buf->isreadable();
}


size_t WvBufferCursorStore::used() const
{
    return length - shift;
}


size_t WvBufferCursorStore::optgettable() const
{
    size_t avail = buf->optpeekable(start + shift);
    assert(avail != 0 || length == shift ||
        ! "buffer cursor operating over invalid region");
    if (avail > length)
        avail = length;
    return avail;
}


const void *WvBufferCursorStore::get(size_t count)
{
    assert(count <= length - shift ||
        ! "attempted to get() more than used()");
    const void *data = buf->peek(start + shift, count);
    shift += count;
    return data;
}


void WvBufferCursorStore::skip(size_t count)
{
    assert(count <= length - shift ||
        ! "attempted to skip() more than used()");
    shift += count;
}


void WvBufferCursorStore::unget(size_t count)
{
    assert(count <= shift ||
        ! "attempted to unget() more than ungettable()");
    shift -= count;
}


size_t WvBufferCursorStore::ungettable() const
{
    return shift;
}


void WvBufferCursorStore::zap()
{
    shift = length;
}


size_t WvBufferCursorStore::peekable(int offset) const
{
    offset += shift;
    offset -= start;
    if (offset < 0 || offset > int(length))
        return 0;
    return length - size_t(offset);
}


size_t WvBufferCursorStore::optpeekable(int offset) const
{
    size_t avail = buf->optpeekable(start + shift + offset);
    assert(avail != 0 || length == shift ||
        ! "buffer cursor operating over invalid region");
    size_t max = peekable(offset);
    if (avail > max)
        avail = max;
    return avail;
}


const void *WvBufferCursorStore::peek(int offset, size_t count)
{
    offset += shift;
    assert((offset >= start && offset - start + count <= length) ||
        ! "attempted to peek() with invalid offset or count");
    return buf->peek(offset, count);
}


bool WvBufferCursorStore::iswritable() const
{
    // check if mutablepeek() is supported
    return buf->iswritable();
}


void *WvBufferCursorStore::mutablepeek(int offset, size_t count)
{
    offset += shift;
    assert((offset >= start && offset - start + count <= length) ||
        ! "attempted to peek() with invalid offset or count");
    return buf->mutablepeek(offset, count);
}
