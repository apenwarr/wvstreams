/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generic buffering API.
 * Please declare specializations in a separate header file,
 * See "wvbuffer.h".
 */
#ifndef __WVBUFFERBASE_H
#define __WVBUFFERBASE_H

#include "wvtraits.h"
#include "wvlinklist.h"
#include <assert.h>
#include <limits.h>

// This value is used internally to signal unlimited space.
// It is merely meant to be as large as possible yet leave enough
// room to accomodate simple arithmetic operations without overflow.
// Clients should NOT check for the presence of this value explicitly.
#define UNLIMITED_SPACE (INT_MAX/2)

/**
 * An abstract generic buffer template.
 * Buffers are simple data queues designed to ease the construction of
 * functions that must generate, consume, or transform large amount of
 * data in pipeline fashion.  Concrete buffer subclases define the actual
 * storage mechanism and queuing machinery.  In addition they may provide
 * additional functionality for accomplishing particular tasks.
 *
 * The base component is split into two parts, WvBufferBaseCommonImpl
 * that defines the common API for all buffer types, and WvBufferBase
 * that allows specializations to be defined to add functionality
 * to the base type.  When passing around buffer objects, you should
 * use the WvBufferBase<T> type rather than WvBufferBaseCommonImpl<T>.
 */
template<class T>
class WvBufferBase;
template<class T>
class WvBufferBaseCommonImpl
{
protected:
    typedef T Elem;
    typedef WvTraits<T> ElemTraits;
    typedef typename ElemTraits::MemOps ElemMemOps;
    typedef WvBufferBase<T> Buffer;

    // discourage copying
    WvBufferBaseCommonImpl(const WvBufferBaseCommonImpl &other) { }

public:
    WvBufferBaseCommonImpl() { }
    virtual ~WvBufferBaseCommonImpl() { }

    /*** Buffer Reading ***/

    /**
     * Returns true if the buffer supports reading.
     */
    virtual bool isreadable() const
        { return true; }
    
    /**
     * Returns the number of elements in the buffer currently
     *   available for reading.
     */
    virtual size_t used() const = 0;

    /**
     * Returns the optimal maximum number of elements in the
     *   buffer currently available for reading without incurring
     *   significant overhead.
     * Invariants:
     *   usedopt() <= used()
     *   usedopt() != 0 if used() != 0
     */
    virtual size_t usedopt() const
        { return used(); }

    /**
     * Reads exactly the specified number of elements and returns
     *   a pointer to a storage location owned by the buffer.
     * The pointer is only valid until the next non-const buffer
     *   member is called.
     * If count == 0, a NULL pointer may be returned.
     * It is an error for count to be greater than used().
     * For best results, call get(...) multiple times with count no
     *   greater than usedopt() each time.
     *
     * After this operation, at least count elements may be ungotten.
     */
    virtual const T *get(size_t count) = 0;

    /**
     * Ungets exactly the specified number of elements by returning
     *   them to the buffer.
     * This operation may always be safely performed with count
     *   less than or equal to that specified in the last get(...)
     *   if no non-const buffer members have been called since then.
     * If count == 0, nothing happens.
     * It is an error for count to be greater than ungettable().
     */
    virtual void unget(size_t count) = 0;

    /**
     * Returns the maximum number of elements that may be ungotten
     *   at this time.
     */
    virtual size_t ungettable() const = 0;

    /**
     * Returns a pointer at the specified offset into the buffer without
     *   actually adjusting the current get() index.
     * If count != NULL, *count will be filled in with the number of
     *   elements in the returned array.  This number will always be
     *   at least 1.
     * If offset is greater than zero, then elements will be returned
     *   beginning with the last one that would be returned on a
     *   get(offset + 1).  (ie. skips 'offset' elements)
     * If offset equals zero, then elements will be returned beginning
     *   with the next one available for get(...).
     * If offset is less than zero, then elements will be returned
     *   beginning with the first one that would be returned on a
     *   get(1) after unget(-offset).
     * It is an error for offset >= used() or offset < -ungettable(),
     *   thus peek(...) may not be called if used() == 0.
     *
     * It may be necessary to repeatedly invoke this function in order
     *   to scan the entire contents of the buffer.
     */
    virtual const T *peek(int offset, size_t *count)
        { return mutablepeek(offset, count); }

    /**
     * Clears the buffer.
     * For many types of buffers, calling zap() will increased the
     *   amount of free space available for writing (see below) by
     *   an amount greater than used().  Hence it is wise to zap()
     *   a buffer just before writing to it to maximize free space.
     *
     * After this operation, used() == 0, and often ungettable() == 0.
     */
    virtual void zap() = 0;

    /**
     * Returns the next element from the buffer.
     * It is an error to invoke this method if used() == 0.
     * 
     * After this operation, at least 1 element may be ungotten.
     */
    T get()
        { return *get(1); }

    /**
     * Peeks the specified element from the buffer.
     * It is an error to invoke this method if used() == 0.
     * See peek(...) for information about offset.
     */
    T peek(int offset)
        { return *peek(offset, NULL); }

    /**
     * Efficiently copies the specified number of elements from the
     *   buffer to the specified UNINITIALIZED storage location
     *   and removes the elements from the buffer.
     * It is an error for count to be greater than used().
     * For best results, call move(...) with a large count.
     * The pointer buf may be NULL only if count == 0.
     *
     * After this operation, an indeterminate number of elements
     *   may be ungotten.
     */
    void move(T *buf, size_t count)
    {
        while (count > 0)
        {
            size_t amount = usedopt();
            assert(amount != 0 ||
                !"attempted to move() more than used()");
            if (amount > count)
                amount = count;
            const T *data = get(amount);
            ElemMemOps::uninit_copy(buf, data, amount);
            buf += amount;
            count -= amount;
        }
    }
    
    /**
     * Efficiently copies the specified number of elements from the
     *   buffer to the specified UNINITIALIZED storage location
     *   but does not remove the elements from the buffer.
     * It is an error for count to be greater than used().
     * For best results, call copy(...) with a large count.
     * The pointer buf may be NULL only if count == 0.
     * See peek(...) for information about offset.
     */
    void copy(T *buf, size_t count, int offset = 0)
    {
        while (count > 0)
        {
            size_t amount;
            const T *data = peek(offset, & amount);
            assert(amount != 0 ||
                !"attempted to copy() with invalid offset");
            if (amount > count)
                amount = count;
            ElemMemOps::uninit_copy(buf, data, amount);
            buf += amount;
            count -= amount;
            offset += amount;
        }
    }
    
    /*** Buffer Writing ***/
    
    /**
     * Returns true if the buffer supports writing.
     */
    virtual bool iswritable() const
        { return true; }
    
    /**
     * Returns the number of elements that the buffer can currently
     *   accept for writing.
     */
    virtual size_t free() const = 0;
    
    /**
     * Returns the optimal maximum number of elements that the
     *   buffer can currently accept for writing without incurring
     *   significant overhead.
     * Invariants:
     *   freeopt() <= free()
     *   freeopt() != 0 if free() != 0
     */
    virtual size_t freeopt() const
        { return free(); }
    
    /**
     * Allocates exactly storage for the specified number of elements
     *   and returns a pointer to an UNINITIALIZED storage location
     *   owned by the buffer.
     * The pointer is only valid until the next non-const buffer
     *   member is called.
     * If count == 0, a NULL pointer may be returned.
     * It is an error for count to be greater than free().
     * For best results, call alloc(...) multiple times with count no
     *   greater than freeopt() each time.
     *
     * After this operation, at least count elements may be unallocated.
     */
    virtual T *alloc(size_t count) = 0;

    /**
     * Unallocates exactly the specified number of elements by removing
     *   them from the buffer.
     * This operation may always be safely performed with count
     *   less than or equal to that specified in the last alloc(...)
     *   or put(...) if no non-const buffer members have been called
     *   since then.
     * If count == 0, nothing happens.
     * It is an error for count to be greater than unallocable().
     */
    virtual void unalloc(size_t count) = 0;

    /**
     * Returns the maximum number of elements that may be unallocated
     *   at this time.
     * For all practical purposes, this number will always be at least
     *   as large as the amount currently in use.  It is provided
     *   primarily for symmetry, but also to handle cases where
     *   buffer reading is not supported by the implementation.
     * Invariants:
     *   unallocable() >= used()
     */
    virtual size_t unallocable() const = 0;
    
    /**
     * Returns a non-const pointer at the specified offset into the
     *   buffer without actually adjusting the current get() index.
     * Other than the fact that the storage is mutable, functions
     *   identically to peek().
     */
    virtual T *mutablepeek(int offset, size_t *count) = 0;
    
    /**
     * Allocates and copies the specified element into the buffer.
     * It is an error to invoke this method if free() == 0.
     * 
     * After this operation, at least 1 element may be unallocated.
     */
    void put(typename ElemTraits::Param value)
        { ElemMemOps::uninit_copy1(alloc(1), value); }

    /**
     * Efficiently copies the specified number of elements from the
     *   specified storage location into newly allocated buffer storage.
     * It is an error for count to be greater than free().
     * For best results, call put(...) with a large count.
     * The pointer buf may be NULL only if count == 0.
     * 
     * After this operation, at least 1 element may be unallocated.
     */
    void put(const T *data, size_t count)
    {
        while (count > 0)
        {
            size_t amount = freeopt();
            assert(amount != 0 || !"attempted to put() more than free()");
            if (amount > count)
                amount = count;
            T *buf = alloc(amount);
            ElemMemOps::uninit_copy(buf, data, amount);
            data += amount;
            count -= amount;
        }
    }

    /**
     * Efficiently copies the specified number of elements from the
     *   specified storage location into a particular offset of
     *   the buffer.
     * If offset <= used() and offset + count > used(), the
     *   remaining data is tacked onto the end of the buffer
     *   with put().
     * It is an error for count to be greater than free() - offset.
     *
     * See mutablepeek() and peek().
     */
    void poke(int offset, const T *data, size_t count)
    {
        int limit = int(used());
        assert(offset <= limit ||
            !"attempted to poke() beyond end of buffer");
        int end = offset + count;
        if (end >= limit)
        {
            size_t tail = end - limit;
            count -= tail;
            put(data + count, tail);
        }
        while (count > 0)
        {
            size_t amount;
            T *buf = mutablepeek(offset, & amount);
            if (amount > count)
                amount = count;
            ElemMemOps::copy(buf, data, amount);
            data += amount;
            count -= amount;
        }
    }
    

    /*** Buffer to Buffer Transfers ***/

    /**
     * Efficiently moves count bytes from the specified buffer into
     *   this one.
     * It is an error for count to be greater than inbuf.used().
     * For best results, call merge(...) with a large count.
     *
     * After this operation, an indeterminate number of elements
     *   may be ungotten from inbuf.
     */
    virtual void merge(Buffer &inbuf, size_t count)
    {
        // move bytes as efficiently as we can using only the public API
        if (count == 0)
            return;
        const T *indata = NULL;
        T *outdata = NULL;
        size_t inavail = 0;
        size_t outavail = 0;
        for (;;)
        {
            if (inavail == 0)
            {
                inavail = inbuf.usedopt();
                assert(inavail != 0 ||
                    !"attempted to merge() more than inbuf.used()");
                if (inavail > count)
                    inavail = count;
                indata = inbuf.get(inavail);
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
                ElemMemOps::uninit_copy(outdata, indata, inavail);
                count -= inavail;
                outavail -= inavail;
                if (count == 0)
                {
                    unalloc(outavail);
                    return;
                }
                outdata += inavail;
                inavail = 0;
            }
            else
            {
                ElemMemOps::uninit_copy(outdata, indata, outavail);
                count -= outavail;
                if (count == 0) return;
                inavail -= outavail;
                indata += outavail;
                outavail = 0;
            }
        }
    }

    /**
     * Efficiently merges an entire buffer's contents into this one.
     * This is a convenience method.  See merge(...) for more details.
     */
    void merge(Buffer &inbuf)
    {
        merge(inbuf, inbuf.used());
    }
};


/**
 * The REAL generic buffer base type.
 * To specialize buffers to add new functionality, declare a template
 * specialization of this type that derives from WvBufferBaseCommonImpl.
 * See WvBufferBaseCommonImpl.
 */
template<class T>
class WvBufferBase : public WvBufferBaseCommonImpl<T>
{
};

/**
 * A statically bound mixin template for buffer implementations that are
 * read-only.  It is an error to attempt to write to a read-only buffer.
 * Note that read-only in this context does not mean the same as "const".
 */
template<class Super>
class WvReadOnlyBufferMixin : public Super
{
public:
    virtual bool iswritable() const
    {
        return false;
    }
    virtual size_t free() const
    {
        return 0;
    }
    virtual size_t freeopt() const
    {
        return 0;
    }
    virtual typename Super::Elem *alloc(size_t count)
    {
        if (count == 0) return NULL;
        assert(! "non-zero alloc() called on non-writable buffer");
        return NULL;
    }
    virtual void unalloc(size_t count)
    {
        if (count == 0) return;
        assert(! "non-zero unalloc() called on non-writable buffer");
    }
    virtual size_t unallocable() const
    {
        return 0;
    }
    virtual typename Super::Elem *mutablepeek(int offset, size_t *count)
    {
        assert(! "mutablepeek() called on non-writable buffer");
        return NULL;
    }
};


/**
 * A statically bound mixin template for buffer implementations that are
 * write-only.  It is an error to attempt to read from a write-only buffer.
 */
template<class Super>
class WvWriteOnlyBufferMixin : public Super
{
public:
    virtual bool isreadable() const
    {
        return false;
    }
    virtual size_t used() const
    {
        return 0;
    }
    virtual size_t usedopt() const
    {
        return 0;
    }
    virtual const typename Super::Elem *get(size_t count)
    {
        if (count == 0) return NULL;
        assert(! "non-zero get() called on non-readable buffer");
        return NULL;
    }
    virtual void unget(size_t count)
    {
        if (count == 0) return;
        assert(! "non-zero unget() called on non-readable buffer");
    }
    virtual size_t ungettable() const
    {
        return 0;
    }
    virtual const typename Super::Elem *peek(int offset, size_t *count)
    {
        assert(! "peek() called on non-readable buffer");
        return NULL;
    }
    virtual void zap()
    {
        // nothing to zap
    }
};


/**
 * A buffer that wraps a pre-allocated array and provides
 * read-write access to its elements.
 */
template<class T>
class WvInPlaceBufferBase : public WvBufferBase<T>
{
protected:
    T *data;
    size_t xsize;
    size_t readidx;
    size_t writeidx;
    bool xautofree;

public:
    /**
     * Creates a new buffer backed by the supplied array.
     *   data  - the array of data to wrap (not copied)
     *   avail - the amount of data available for reading
     *   size  - the total size of the array
     *   auto_free - if true, the buffer is freed on delete
     */
    WvInPlaceBufferBase(T *_data, size_t _avail, size_t _size,
        bool _autofree = false) : data(NULL)
        { reset(_data, _avail, _size, _autofree); }

    /**
     * Creates a new empty buffer backed by a new array.
     *   size  - the total size of the array to create
     */
    WvInPlaceBufferBase(size_t _size) : data(NULL)
        { reset(new T[_size], 0, _size, true); }

    /**
     * Creates a new empty buffer with no backing array.
     */
    WvInPlaceBufferBase() : data(NULL)
        { reset(NULL, 0, 0, false); }
        
    virtual ~WvInPlaceBufferBase()
    {
        if (xautofree)
            delete[] data;
    }

    /**
     * Returns the underlying array pointer.
     */
    T *ptr() const
        { return data; }

    /**
     * Returns the total size of the buffer.
     */
    size_t size() const
        { return xsize; }

    /**
     * Returns the autofree flag.
     */
    bool autofree() const
        { return xautofree; }

    /**
     * Sets or clears the auto_free flag.
     */
    void setautofree(bool _autofree)
        { xautofree = _autofree; }

    /**
     * Resets the underlying buffer pointer and properties.
     * If the old and new buffer pointers differ and the old buffer
     * was specified as auto_free, the old buffer is destroyed.
     */
    void reset(T *_data, size_t _avail, size_t _size, bool _autofree = false)
    {
        assert(_data != NULL || _avail == 0);
        assert(_avail <= _size);
        if (_data != data && xautofree)
            delete[] data;
        data = _data;
        xsize = _size;
        readidx = 0;
        writeidx = _avail;
        xautofree = _autofree;
    }

    /*** Overridden Members ***/
    virtual size_t used() const
    {
        return writeidx - readidx;
    }
    virtual const T *get(size_t count)
    {
        assert(count <= writeidx - readidx ||
            !"attempted to get() more than used()");
        const T *tmpptr = data + readidx;
        readidx += count;
        return tmpptr;
    }
    virtual void unget(size_t count)
    {
        assert(count <= readidx ||
            !"attempted to unget() more than ungettable()");
        readidx -= count;
    }
    virtual size_t ungettable() const
    {
        return readidx;
    }
    virtual void zap()
    {
        readidx = writeidx = 0;
    }
    virtual size_t free() const
    {
        return xsize - writeidx;
    }
    virtual T *alloc(size_t count)
    {
        assert(count <= xsize - writeidx ||
            !"attempted to alloc() more than free()");
        T *tmpptr = data + writeidx;
        writeidx += count;
        return tmpptr;
    }
    virtual void unalloc(size_t count)
    {
        assert(count <= writeidx - readidx ||
            !"attempted to unalloc() more than unallocable()");
        writeidx -= count;
    }
    virtual size_t unallocable() const
    {
        return writeidx - readidx;
    }
    virtual T *mutablepeek(int offset, size_t *count)
    {
        if (offset < 0)
            assert(size_t(-offset) <= readidx ||
                !"attempted to peek() with invalid offset");
        else
            assert(size_t(offset) < writeidx - readidx ||
                !"attempted to peek() with invalid offset");
        if (count)
            *count = writeidx - readidx - offset;
        return data + readidx + offset;
    }
};


/**
 * A buffer that wraps a pre-allocated array and provides
 * read-only access to its elements.
 */
template<class T>
class WvConstInPlaceBufferBase :
    public WvReadOnlyBufferMixin<WvBufferBase<T> >
{
protected:
    const T *data;
    size_t avail;
    size_t readidx;

public:
    /**
     * Creates a new buffer.
     *   data  - the array of data to wrap (not copied)
     *   avail - the amount of data available for reading
     */
    WvConstInPlaceBufferBase(const T *_data, size_t _avail)
        { reset(_data, _avail); }

    /**
     * Creates a new empty buffer with no backing array.
     */
    WvConstInPlaceBufferBase()
        { reset(NULL, 0); }

    virtual ~WvConstInPlaceBufferBase() { }

    /**
     * Returns the underlying array pointer.
     */
    const T *ptr() const
        { return data; }

    /**
     * Resets the underlying buffer pointer and properties.
     */
    void reset(const T *_data, size_t _avail)
    {
        assert(_data != NULL || _avail == 0);
        data = _data;
        avail = _avail;
        readidx = 0;
    }

    /*** Overridden Members ***/
    virtual size_t used() const
    {
        return avail - readidx;
    }
    virtual const T *get(size_t count)
    {
        assert(count <= avail - readidx ||
            !"attempted to get() more than used()");
        const T *ptr = data + readidx;
        readidx += count;
        return ptr;
    }
    virtual void unget(size_t count)
    {
        assert(count <= readidx ||
            !"attempted to unget() more than ungettable()");
        readidx -= count;
    }
    virtual size_t ungettable() const
    {
        return readidx;
    }
    virtual const T *peek(int offset, size_t *count)
    {
        if (offset < 0)
            assert(size_t(-offset) <= readidx ||
                !"attempted to peek() with invalid offset");
        else
            assert(size_t(offset) < avail - readidx ||
                !"attempted to peek() with invalid offset");
        if (count)
            *count = avail - readidx - offset;
        return data + readidx + offset;
    }
    virtual void zap()
    {
        readidx = avail = 0;
    }
};


/**
 * A buffer built out of a list of other buffers linked together.
 * Buffers may be appended or prepended to the list at any time, at
 * which point they act as slaves for the master buffer.  Slaves may
 * be expunged from the list at any time when the master buffer
 * determines that they are of no further use.
 */
template<class T>
class WvLinkedBufferBase : public WvBufferBase<T>
{
protected:
    DeclareWvList(Buffer);
    BufferList list;
    size_t totalused;

public:
    /**
     * Creates a new buffer.
     */
    WvLinkedBufferBase() :
        totalused(0) { }

    /**
     * Appends a buffer to the list.
     */
    virtual void append(Buffer *buffer, bool autofree)
    {
        list.append(buffer, autofree);
        totalused += buffer->used();
    }

    /**
     * Prepends a buffer to the list.
     */
    virtual void prepend(Buffer *buffer, bool autofree)
    {
        list.prepend(buffer, autofree);
        totalused += buffer->used();
    }

    /**
     * Unlinks a buffer from the list.
     */
    virtual void unlink(Buffer *buffer)
    {
        BufferList::Iter it(list);
        if (it.find(buffer))
        {
            totalused -= buffer->used();
            it.unlink(); // do not recycle the buffer
        }
    }

    /**
     * Returns the number of buffers in the list.
     */
    virtual size_t numbuffers()
    {
        return list.count();
    }

    /*** Overridden Members ***/
    virtual size_t used() const
    {
        return totalused;
    }
    virtual size_t usedopt() const
    {
        size_t count;
        BufferList::Iter it(list);
        for (it.rewind(); it.next(); )
            if ((count = it->usedopt()) != 0)
                return count;
        return 0;
    }
    virtual const T *get(size_t count)
    {
        if (count == 0)
            return NULL;
        totalused -= count;
        // search for first non-empty buffer
        Buffer *buf;
        size_t availused;
        for (;;)
        {
            assert(! list.isempty() ||
                !"attempted to get() more than used()");
            buf = list.first();
            availused = buf->used();
            if (availused != 0) break;
            // unlink the leading empty buffer
            BufferList::Iter it(list);
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
        BufferList::Iter it(list);
        it.rewind(); it.next();
        for (;;)
        {
            assert(it.next() || !"attempted to get() more than used()");
            Buffer *itbuf = it.ptr();
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
    virtual void unget(size_t count)
    {
        if (count == 0)
            return;
        assert(! list.isempty() ||
            !"attempted to unget() more than ungettable()");
        totalused += count;
        list.first()->unget(count);
    }
    virtual size_t ungettable() const
    {
        if (list.isempty())
            return 0;
        return list.first()->ungettable();
    }
    virtual void zap()
    {
        totalused = 0;
        BufferList::Iter it(list);
        for (it.rewind(); it.next(); )
            do_xunlink(it);
    }
    virtual size_t free() const
    {
        if (! list.isempty())
            return list.last()->free();
        return 0;
    }
    virtual size_t freeopt() const
    {
        if (! list.isempty())
            return list.last()->freeopt();
        return 0;
    }
    virtual T *alloc(size_t count)
    {
        if (count == 0)
            return NULL;
        assert(! list.isempty() ||
            !"attempted to alloc() more than free()");
        totalused += count;
        return list.last()->alloc(count);
    }
    virtual void unalloc(size_t count)
    {
        totalused -= count;
        while (count > 0)
        {
            assert(! list.isempty() ||
                !"attempted to unalloc() more than unallocable()");
            Buffer *buf = list.last();
            size_t avail = buf->unallocable();
            if (count < avail)
            {
                buf->unalloc(count);
                break;
            }
            BufferList::Iter it(list);
            it.find(buf);
            do_xunlink(it);
            count -= avail;
        }
    }
    virtual size_t unallocable() const
    {
        return totalused;
    }
    virtual T *mutablepeek(int offset, size_t *count)
    {
        if (offset < 0)
        {
            assert(size_t(-offset) <= ungettable() ||
                !"attempted to peek() with invalid offset");
            return list.first()->mutablepeek(offset, count);
        }
        assert(size_t(offset) < used() ||
            !"attempted to peek() with invalid offset");
            
        // search for the buffer that contains the offset
        BufferList::Iter it(list);
        it.rewind();
        Buffer *buf;
        for (;;) {
            assert(it.next());
            buf = it.ptr();
            size_t len = buf->used();
            if (size_t(offset) < len)
                break;
            offset -= len;
        }
        return buf->mutablepeek(offset, count);
    }

protected:
    /**
     * Called when a new buffer must be allocated to coalesce chunks.
     *   minsize : the minimum size for the new buffer
     */
    virtual Buffer *newbuffer(size_t minsize)
    {
        return new WvInPlaceBufferBase<T>(minsize);
    }

    /**
     * Called when a buffer with autofree is removed from the list.
     * This function is not called during object destruction.
     */
    virtual void recyclebuffer(Buffer *buffer)
    {
        delete buffer;
    }

private:
    // unlinks and recycles the buffer pointed at by the iterator
    void do_xunlink(BufferList::Iter &it)
    {
        Buffer * buf = it.ptr();
        bool autofree = it.link->auto_free;
        it.link->auto_free = false;
        it.xunlink();
        if (autofree)
            recyclebuffer(buf);
    }
};


/**
 * A buffer that dynamically grows and shrinks based on demand.
 */
template<class T>
class WvDynamicBufferBase : public WvLinkedBufferBase<T>
{
    typedef WvLinkedBufferBase<T> Super;
    size_t minalloc;
    size_t maxalloc;
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
    BufferList pool;
#endif
    
public:
    /**
     * Creates a new buffer.
     *   minalloc - A tuning parameter that specifies the minimum
     *              amount of space to allocate for a new buffer.
     *   maxalloc - A tuning parameter that specifies the maximum
     *              amount of space to allocate for a new buffer
     *              before reverting to a linear growth pattern.
     *              Larger buffers may be allocated to coalesce
     *              chunks during a get().
     *
     * Note that these parameters control the number of entries to
     * be allocated.  The actual number of bytes allocated will vary
     * in proportion with sizeof(T).
     */
    WvDynamicBufferBase(size_t minalloc = 1024,
        size_t maxalloc = 1048576) :
        minalloc(minalloc), maxalloc(maxalloc)
    {
        assert(maxalloc >= minalloc);
    }

    /*** Overridden Members ***/
    virtual size_t free() const
    {
        return UNLIMITED_SPACE;
    }
    virtual size_t freeopt() const
    {
        size_t avail = Super::freeopt();
        if (avail == 0)
            avail = UNLIMITED_SPACE;
        return avail;
    }
    virtual T *alloc(size_t count)
    {
        if (count > Super::free())
        {
            Buffer *buf = newbuffer(count);
            append(buf, true);
        }
        return Super::alloc(count);
    }

protected:
    virtual Buffer *newbuffer(size_t minsize)
    {
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
        // try to find a suitable buffer in the pool
        BufferList::Iter it(pool);
        for (it.rewind(); it.next(); )
        {
            Buffer *buf = it.ptr();
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
        return Super::newbuffer(size);
    }
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
    virtual void recyclebuffer(Buffer *buffer)
    {
        // add the buffer to the pool
        buffer->zap();
        pool.append(buffer, true);
    }
#endif
};


/**
 * A buffer that is always empty.
 */
template<class T>
class WvEmptyBufferBase : public WvWriteOnlyBufferMixin<
    WvReadOnlyBufferMixin<WvBufferBase<T> > >
{
};


/**
 * A buffer that provides a view over a window of another buffer.
 * The underlying buffer's get() position is not affected by
 * reading from this buffer.
 */
template<class T>
class WvBufferCursorBase : public WvBufferBase<T>
{
protected:
    Buffer *buf;
    int start;
    int end;
    int offset;
    WvDynamicBufferBase<T> tmpbuf;

public:
    WvBufferCursorBase(Buffer *_buf, int _start, size_t _length) :
        buf(_buf), start(_start), end(_start + _length), offset(0)
    {
    }

    /*** Overridden Members ***/
    virtual size_t used() const
    {
        int pos = getpos();
        return end - pos;
    }
    virtual size_t usedopt() const
    {
        int pos = getpos();
        size_t avail;
        if (pos == 0)
            avail = buf->usedopt();
        else if (pos >= end)
            avail = 0;
        else
            buf->peek(pos, & avail);
        size_t maxavail = size_t(end - start);
        if (avail > maxavail)
            avail = maxavail;
        return avail;
    }
    virtual const T *get(size_t count)
    {
        if (count == 0)
            return NULL;
        tmpbuf.zap();
        int pos = getpos();
        size_t avail;
        const T *ptr = buf->peek(pos, & avail);
        if (avail < count)
        {
            T *nptr = tmpbuf.alloc(count);
            buf->copy(nptr, count, pos);
            ptr = nptr;
        }
        offset += count;
        return ptr;
    }
    virtual void unget(size_t count)
    {
        assert(count <= size_t(offset) ||
            !"attempted to unget() more than ungettable()");
        offset -= count;
    }
    virtual size_t ungettable() const
    {
        return offset;
    }
    virtual void zap()
    {
        start = end = offset = 0;
    }
    virtual size_t free() const
    {
        return 0;
    }
    virtual T *alloc(size_t count)
    {
        assert(count == 0 ||
            !"attempted to alloc() more than free()");
        return NULL;
    }
    virtual void unalloc(size_t count)
    {
        assert(count == 0 ||
            !"attempted to unalloc() more than unallocable()");
    }
    virtual size_t unallocable() const
    {
        return 0;
    }
    virtual T *mutablepeek(int offset, size_t *count)
    {
        int pos = getpos();
        size_t avail;
        T *ptr = buf->mutablepeek(pos, & avail);
        if (count)
        {
            size_t maxavail = size_t(end - start);
            if (avail > maxavail)
                avail = maxavail;
            *count = avail;
        }
        return ptr;
    }

protected:
    int getpos() const
    {
        int pos = start + offset;
        assert(pos >= 0 && size_t(pos) <= buf->used() ||
            size_t(pos) >= buf->ungettable() ||
            !"attempted to operate on buffer cursor over invalid region");
        return pos;
    }
};

#endif // __WVBUFFERBASE_H
