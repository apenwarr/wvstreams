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

#include "wvbufferstore.h"
#include "wvtraits.h"

template<class T>
class WvBufferBase;

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
 *
 * @see WvBufferBase<T>
 */
template<class T>
class WvBufferBaseCommonImpl
{
protected:
    typedef T Elem;
    typedef WvTraits<T> ElemTraits;
    typedef WvBufferBase<T> Buffer;

    WvBufferStore *store;
    
    // discourage copying
    WvBufferBaseCommonImpl(const WvBufferBaseCommonImpl &other) { }

protected:
    /**
     * Initializes the buffer.
     * <p>
     * Note: Does not take ownership of the storage object.
     * </p>
     *
     * @param store the low-level storage object
     */
    WvBufferBaseCommonImpl(WvBufferStore *store) :
        store(store) { }

public:
    /**
     * Destroys the buffer.
     */
    virtual ~WvBufferBaseCommonImpl() { }

    /**
     * Returns a pointer to the underlying storage class object.
     *
     * @return the low-level storage class object pointer, non-null
     */
    inline WvBufferStore *getstore()
    {
        return store;
    }

    /*** Buffer Reading ***/

    /**
     * Returns true if the buffer supports reading.
     *
     * @return true if reading is supported
     */
    inline bool isreadable() const
    {
        return store->isreadable();
    }
    
    /**
     * Returns the number of elements in the buffer currently
     * available for reading.
     *
     * @return the number of elements
     */
    inline size_t used() const
    {
        return store->used() / sizeof(Elem);
    }

    /**
     * Returns the optimal maximum number of elements in the
     * buffer currently available for reading without incurring
     * significant overhead.
     * <p>
     * Invariants:
     * <ul>
     * <li>optgettable() <= used()</li>
     * <li>optgettable() != 0 if used() != 0</li>
     * </ul></p>
     *
     * @return the number of elements
     */
    size_t optgettable() const
    {
        size_t avail = store->optgettable();
        size_t elems = avail / sizeof(Elem);
        if (elems != 0) return elems;
        return avail != 0 && store->used() >= sizeof(Elem) ? 1 : 0;
    }

    /**
     * Reads exactly the specified number of elements and returns
     * a pointer to a storage location owned by the buffer.
     * <p>
     * The pointer is only valid until the next non-const buffer
     * member is called. eg. alloc(size_t)
     * </p><p>
     * If count == 0, a NULL pointer may be returned.
     * </p><p>
     * It is an error for count to be greater than used().
     * </p><p>
     * For best results, call this function multiple times with
     * count no greater than optgettable() each time.
     * </p><p>
     * After this operation, at least count elements may be ungotten.
     * </p>
     *
     * @param count the number of elements
     * @return the element storage pointer
     */
    inline const T *get(size_t count)
    {
        return static_cast<const T*>(
            store->get(count * sizeof(Elem)));
    }

    /**
     * Ungets exactly the specified number of elements by returning
     * them to the buffer for subsequent reads.
     * <p>
     * This operation may always be safely performed with count
     * less than or equal to that specified in the last get(size_t)
     * if no non-const buffer members have been called since then.
     * </p><p>
     * If count == 0, nothing happens.
     * </p><p>
     * It is an error for count to be greater than ungettable().
     * </p>
     *
     * @param count the number of elements
     */
    inline void unget(size_t count)
    {
        store->unget(count * sizeof(Elem));
    }

    /**
     * Returns the maximum number of elements that may be ungotten
     * at this time.
     *
     * @return the number of elements
     */
    inline size_t ungettable() const
    {
        return store->ungettable() / sizeof(Elem);
    }

    /**
     * Returns a pointer into the buffer at the specified offset
     * without actually adjusting the current get() index.
     * <p>
     * If count != NULL, *count will be filled in with the number of
     * elements in the returned array.  This number will always be
     * at least mincount.
     * </p><p>
     * If offset is greater than zero, then elements will be returned
     * beginning with the last one that would be returned by
     * get(offset + 1).  (ie. skips 'offset' elements)
     * </p><p>
     * If offset equals zero, then elements will be returned beginning
     * with the next one available for get(size_t).
     * </p><p>
     * If offset is less than zero, then elements will be returned
     * beginning with the first one that would be returned on a
     * get(1) after unget(-offset).
     * </p><p>
     * It is an error for offset + mincount > used() or
     * for offset < -ungettable(), thus this function may not be
     * called if used() == 0.
     * </p><p>
     * It is an error for mincount == 0.
     * </p><p>
     * It may be necessary to repeatedly invoke this function in order
     * to scan the entire contents of the buffer.  For maximum
     * efficiency, choose a mincount as small as possible.
     * </p>
     *
     * @param offset the buffer offset
     * @param count pointer to the returned element count
     * @param mincount the minimum number of elements to peek, default 1
     * @return the element storage pointer
     */
    inline const T *peek(int offset, size_t *count,
        size_t mincount = 1)
    {
        const T *data = static_cast<const T*>(store->peek(
            offset * sizeof(Elem), count, mincount * sizeof(Elem)));
        if (count) *count /= sizeof(Elem);
        return data;
    }

    /**
     * Clears the buffer.
     * <p>
     * For many types of buffers, calling zap() will increased the
     * amount of free space available for writing (see below) by
     * an amount greater than used().  Hence it is wise to zap()
     * a buffer just before writing to it to maximize free space.
     * </p><p>
     * After this operation, used() == 0, and often ungettable() == 0.
     * </p>
     */
    inline void zap()
    {
        store->zap();
    }

    /**
     * Reads the next element from the buffer.
     * <p>
     * It is an error to invoke this method if used() == 0.
     * </p><p>
     * After this operation, at least 1 element may be ungotten.
     * </p>
     *
     * @return the element
     * @see get(size_t)
     */
    inline T get()
    {
        return *get(1);
    }

    /**
     * Returns the element at the specified offset in the buffer.
     * <p>
     * It is an error to invoke this method if used() == 0.
     * </p>
     *
     * @param offset the offset, default 0
     * @return the element
     * @see peek(int, size_t*, size_t)
     */
    inline T peek(int offset = 0)
    {
        return *peek(offset * sizeof(Elem), NULL);
    }

    /**
     * Efficiently copies the specified number of elements from the
     * buffer to the specified UNINITIALIZED storage location
     * and removes the elements from the buffer.
     * <p>
     * It is an error for count to be greater than used().
     * </p><p>
     * For maximum efficiency, choose as large a count as possible.
     * </p><p>
     * The pointer buf may be NULL only if count == 0.
     * </p><p>
     * After this operation, an indeterminate number of elements
     * may be ungotten.
     * </p>
     *
     * @param buf the buffer that will receive the elements
     * @param count the number of elements
     * @see get(size_t)
     */
    inline void move(T *buf, size_t count)
    {
        store->move(buf, count * sizeof(Elem));
    }
    
    /**
     * Efficiently copies the specified number of elements from the
     * buffer to the specified UNINITIALIZED storage location
     * but does not remove the elements from the buffer.
     * <p>
     * It is an error for count to be greater than used().
     * </p><p>
     * For maximum efficiency, choose as large a count as possible.
     * </p><p>
     * The pointer buf may be NULL only if count == 0.
     * </p>
     *
     * @param buf the buffer that will receive the elements
     * @param count the number of elements
     * @param offset the buffer offset, default 0
     * @see peek(int, size_t*, size_t)
     */
    inline void copy(T *buf, size_t count, int offset = 0)
    {
        store->copy(buf, count * sizeof(Elem), offset * sizeof(Elem));
    }
    
    /*** Buffer Writing ***/
    
    /**
     * Returns true if the buffer supports writing.
     *
     * @return true if writing is supported
     */
    inline bool iswritable() const
    {
        return true;
    }
    
    /**
     * Returns the number of elements that the buffer can currently
     * accept for writing.
     * 
     * @return the number of elements
     */
    inline size_t free() const
    {
        return store->free() / sizeof(Elem);
    }
    
    /**
     * Returns the optimal maximum number of elements that the
     * buffer can currently accept for writing without incurring
     * significant overhead.
     * <p>
     * Invariants:
     * <ul>
     * <li>optallocable() <= free()</li>
     * <li>optallocable() != 0 if free() != 0</li>
     * </ul></p>
     *
     * @return the number of elements
     */
    size_t optallocable() const
    {
        size_t avail = store->optallocable();
        size_t elems = avail / sizeof(Elem);
        if (elems != 0) return elems;
        return avail != 0 && store->free() >= sizeof(Elem) ? 1 : 0;
    }
    
    /**
     * Allocates exactly the specified number of elements and returns
     * a pointer to an UNINITIALIZED storage location owned by the
     * buffer.
     * <p>
     * The pointer is only valid until the next non-const buffer
     * member is called. eg. alloc(size_t)
     * </p><p>
     * If count == 0, a NULL pointer may be returned.
     * </p><p>
     * It is an error for count to be greater than free().
     * </p><p>
     * For best results, call this function multiple times with
     * count no greater than optallocable() each time.
     * </p><p>
     * After this operation, at least count elements may be unallocated.
     * </p>
     *
     * @param count the number of elements
     * @return the element storage pointer
     */
    inline T *alloc(size_t count)
    {
        return static_cast<T*>(store->alloc(count * sizeof(Elem)));
    }

    /**
     * Unallocates exactly the specified number of elements by removing
     * them from the buffer and releasing their storage.
     * <p>
     * This operation may always be safely performed with count
     * less than or equal to that specified in the last alloc(size_t)
     * or put(const T*, size_t) if no non-const buffer members have
     * been called since then.
     * </p><p>
     * If count == 0, nothing happens.
     * </p><p>
     * It is an error for count to be greater than unallocable().
     * </p>
     *
     * @param count the number of elements
     */
    inline void unalloc(size_t count)
    {
        return store->unalloc(count * sizeof(Elem));
    }

    /**
     * Returns the maximum number of elements that may be unallocated
     * at this time.
     * <p>
     * For all practical purposes, this number will always be at least
     * as large as the amount currently in use.  It is provided
     * primarily for symmetry, but also to handle cases where
     * buffer reading (hence used()) is not supported by the
     * implementation.
     * </p><p>
     * Invariants:
     * <ul>
     * <li>unallocable() >= used()</li>
     * </ul></p>
     *
     * @return the number of elements
     */
    inline size_t unallocable() const
    {
        return store->unallocable() / sizeof(Elem);
    }
    
    /**
     * Returns a non-const pointer info the buffer at the specified
     * offset without actually adjusting the current get() index.
     * <p>
     * Other than the fact that the storage is mutable, functions
     * identically to peek(int, size_t*, size_t).
     * </p>
     *
     * @param offset the buffer offset
     * @param count pointer to the returned element count
     * @param mincount the minimum number of elements to peek, default 1
     * @return the element storage pointer
     * @see peek(int, size_t*, size_t)
     */
    inline T *mutablepeek(int offset, size_t *count,
        size_t mincount = 1)
    {
        T *data = static_cast<T*>(store->mutablepeek(
            offset * sizeof(Elem), count, mincount * sizeof(Elem)));
        if (count) *count /= sizeof(Elem);
        return data;
    }
    
    /**
     * Writes the specified number of elements from the specified
     * storage location into the buffer at its tail.
     * <p>
     * It is an error for count to be greater than free().
     * </p><p>
     * For maximum efficiency, choose as large a count as possible.
     * </p><p>
     * The pointer buf may be NULL only if count == 0.
     * </p><p>
     * After this operation, at least count elements may be unallocated.
     * </p>
     *
     * @param data the buffer that contains the elements
     * @param count the number of elements
     * @see alloc(size_t)
     */
    inline void put(const T *data, size_t count)
    {
        store->put(data, count * sizeof(Elem));
    }

    /**
     * Efficiently copies the specified number of elements from the
     * specified storage location into the buffer at a particular
     * offset.
     * <p>
     * If offset <= used() and offset + count > used(), the
     * remaining data is simply tacked onto the end of the buffer
     * with put().
     * </p><p>
     * It is an error for count to be greater than free() - offset.
     * </p>
     *
     * @param data the buffer that contains the elements
     * @param count the number of elements
     * @param offset the buffer offset, default 0
     * @see mutablepeek(int, size_t*, size_t)
     * @see put(const T*, size_t)
     */
    inline void poke(const T *data, size_t count, int offset = 0)
    {
        store->poke(data, count * sizeof(Elem), offset * sizeof(Elem));
    }

    /**
     * Writes the element into the buffer at its tail.
     * <p>
     * It is an error to invoke this method if free() == 0.
     * </p><p>
     * After this operation, at least 1 element may be unallocated.
     * </p>
     *
     * @param valid the element
     * @see put(const T*, size_t)
     */
    inline void put(typename ElemTraits::Param value)
    {
        store->fastput(& value, sizeof(Elem));
    }

    /**
     * Writes the element into the buffer at the specified offset.
     * <p>
     * It is an error to invoke this method if free() == 0.
     * </p><p>
     * After this operation, at least 1 element may be unallocated.
     * </p>
     *
     * @param value the element
     * @param offset the buffer offset
     * @see poke(const T*, size_t, int)
     */
    inline void poke(typename ElemTraits::Param value, int offset = 0)
    {
        poke(& value, 1, offset);
    }


    /*** Buffer to Buffer Transfers ***/

    /**
     * Efficiently moves count bytes from the specified buffer into
     * this one.  In some cases, this may be a zero-copy operation.
     * <p>
     * It is an error for count to be greater than inbuf.used().
     * </p><p>
     * For maximum efficiency, choose as large a count as possible.
     * </p><p>
     * After this operation, an indeterminate number of elements
     * may be ungotten from inbuf.
     * </p>
     *
     * @param inbuf the buffer from which to read
     * @param count the number of elements
     */
    inline void merge(Buffer &inbuf, size_t count)
    {
        store->merge(*inbuf.store, count * sizeof(Elem));
    }

    /**
     * Efficiently merges the entire contents of a buffer into this one.
     *
     * @param inbuf the buffer from which to read
     * @see merge(Buffer &, size_t)
     */
    inline void merge(Buffer &inbuf)
    {
        merge(inbuf, inbuf.used());
    }
};



/**
 * The REAL generic buffer base type.
 * To specialize buffers to add new functionality, declare a template
 * specialization of this type that derives from WvBufferBaseCommonImpl.
 *
 * @see WvBufferBaseCommonImpl<T>
 */
template<class T>
class WvBufferBase : public WvBufferBaseCommonImpl<T>
{
    WvBufferBase(WvBufferStore *store) :
        WvBufferBaseCommonImpl<T>(store) { }
};



/**
 * A buffer that wraps a pre-allocated array and provides
 * read-write access to its elements.
 */
template<class T>
class WvInPlaceBufferBase : public WvBufferBase<T>
{
protected:
    WvInPlaceBufferStore mystore;

public:
    /**
     * Creates a new buffer backed by the supplied array.
     *
     * @param _data the array of data to wrap
     * @param _avail the amount of data available for reading
     * @param _size the size of the array
     * @param _autofree if true, the array will be freed when discarded
     */
    WvInPlaceBufferBase(T *_data, size_t _avail, size_t _size,
        bool _autofree = false) :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), _data, _avail * sizeof(Elem),
            _size * sizeof(Elem), _autofree) { }

    /**
     * Creates a new empty buffer backed by a new array.
     *
     * @param _size the size of the array
     */
    WvInPlaceBufferBase(size_t _size) :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), _size * sizeof(Elem)) { }

    /**
     * Creates a new empty buffer with no backing array.
     */
    WvInPlaceBufferBase() :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), NULL, 0, 0, false) { }

    /**
     * Destroys the buffer.
     * <p>
     * Frees the underlying array if autofree().
     * </p>
     */
    virtual ~WvInPlaceBufferBase() { }

    /**
     * Returns the underlying array pointer.
     *
     * @return the element pointer
     */
    inline T *ptr() const
    {
        return static_cast<T*>(mystore.ptr());
    }

    /**
     * Returns the total size of the buffer.
     *
     * @return the number of elements
     */
    inline size_t size() const
    {
        return mystore.size() / sizeof(Elem);
    }

    /**
     * Returns the autofree flag.
     *
     * @return the autofree flag
     */
    inline bool autofree() const
    {
        return mystore.autofree();
    }

    /**
     * Sets or clears the auto_free flag.
     *
     * @param _autofree if true, the array will be freed when discarded
     */
    inline void setautofree(bool _autofree)
    {
        mystore.setautofree(_autofree);
    }

    /**
     * Resets the underlying buffer pointer and properties.
     * <p>
     * If the old and new buffer pointers differ and the old buffer
     * was specified as auto_free, the old buffer is destroyed.
     * </p>
     * @param _data the array of data to wrap
     * @param _avail the amount of data available for reading
     * @param _size the size of the array
     * @param _autofree if true, the array will be freed when discarded
     */
    inline void reset(T *_data, size_t _avail, size_t _size,
        bool _autofree = false)
    {
        mystore.reset(_data, _avail * sizeof(Elem),
            _size * sizeof(Elem), _autofree);
    }

    /**
     * Sets the amount of available data using the current buffer
     * and resets the read index to the beginning of the buffer.
     *
     * @param _avail the amount of data available for reading
     */
    inline void setavail(size_t _avail)
    {
        mystore.setavail(_avail * sizeof(Elem));
    }
};



/**
 * A buffer that wraps a pre-allocated array and provides
 * read-only access to its elements.
 */
template<class T>
class WvConstInPlaceBufferBase : public WvBufferBase<T>
{
protected:
    WvConstInPlaceBufferStore mystore;

public:
    /**
     * Creates a new buffer backed by the supplied array.
     *
     * @param _data the array of data to wrap
     * @param _avail the amount of data available for reading
     */
    WvConstInPlaceBufferBase(const T *_data, size_t _avail) :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), _data, _avail * sizeof(Elem)) { }

    /**
     * Creates a new empty buffer with no backing array.
     */
    WvConstInPlaceBufferBase() :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), NULL, 0) { }

    /**
     * Destroys the buffer.
     * <p>
     * Never frees the underlying array.
     * </p>
     */
    virtual ~WvConstInPlaceBufferBase() { }

    /**
     * Returns the underlying array pointer.
     *
     * @return the element pointer
     */
    inline const T *ptr() const
    {
        return static_cast<const T*>(mystore.ptr());
    }

    /**
     * Resets the underlying buffer pointer and properties.
     * <p>
     * Never frees the old buffer.
     * </p>
     *
     * @param _data the array of data to wrap
     * @param _avail the amount of data available for reading
     */
    inline void reset(const T *_data, size_t _avail)
    {
        mystore.reset(_data, _avail * sizeof(Elem));
    }

    /**
     * Sets the amount of available data using the current buffer
     * and resets the read index to the beginning of the buffer.
     *
     * @param _avail the amount of data available for reading
     */
    inline void setavail(size_t _avail)
    {
        mystore.setavail(_avail * sizeof(Elem));
    }
};



/**
 * A buffer that dynamically grows and shrinks based on demand.
 */
template<class T>
class WvDynamicBufferBase : public WvBufferBase<T>
{
protected:
    WvDynamicBufferStore mystore;
    
public:
    /**
     * Creates a new buffer.
     * <p>
     * Provides some parameters for tuning response to buffer
     * growth.
     * </p>
     * @param _minalloc the minimum number of elements to allocate
     *      at once when creating a new internal buffer segment
     * @param _maxalloc the maximum number of elements to allocate
     *      at once when creating a new internal buffer segment
     *      before before reverting to a linear growth pattern
     */
    WvDynamicBufferBase(size_t _minalloc = 1024,
        size_t _maxalloc = 1048576) :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), _minalloc * sizeof(Elem),
            _maxalloc * sizeof(Elem)) { }

    /**
     * Returns the number of buffer stores in the list.
     *
     * @return the number of buffers
     */
    inline size_t numbuffers()
    {
        return mystore.numbuffers();
    }
};



/**
 * A buffer that is always empty.
 */
template<class T>
class WvEmptyBufferBase : public WvBufferBase<T>
{
protected:
    WvEmptyBufferStore mystore;

public:
    /**
     * Creates a new buffer.
     */
    WvEmptyBufferBase() :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem)) { }
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
    WvBufferCursorStore mystore;

public:
    /**
     * Creates a new buffer.
     *
     * @param _buf a pointer to the buffer to be wrapped
     * @param _start the buffer offset of the window start position
     * @param _length the length of the window
     */
    WvBufferCursorBase(WvBufferBase<T> *_buf, int _start,
        size_t _length) :
        WvBufferBase<T>(& mystore),
        mystore(sizeof(Elem), _buf->getstore(),
            _start * sizeof(Elem), _length * sizeof(Elem)) { }
};

#endif // __WVBUFFERBASE_H
