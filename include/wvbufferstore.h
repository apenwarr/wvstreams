/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines basic buffer storage classes.
 * These are not intended for use directly by clients.
 * See "wvbufferbase.h" for the public API.
 */
#ifndef __WVBUFFERSTORE_H
#define __WVBUFFERSTORE_H

#include "wvlinklist.h"
#include <limits.h>

/**
 * This value is used internally to signal unlimited free space.
 * It is merely meant to be as large as possible yet leave enough
 * room to accomodate simple arithmetic operations without overflow.
 * Clients should NOT check for the presence of this value explicitly.
 */
#define UNLIMITED_FREE_SPACE (INT_MAX/2)

/**
 * The abstract buffer storage base class.
 */
class WvBufferStore
{
    // discourage copying
    explicit WvBufferStore(const WvBufferStore &other) { }

protected:
    // the suggested granularity
    int granularity;

    /**
     * Creates a new buffer.
     * @param _granularity the suggested granularity for data allocation
     *        and alignment purposes
     */
    explicit WvBufferStore(int _granularity);
    
public:
    virtual ~WvBufferStore() { }

    /*** Buffer Reading ***/
    
    virtual bool isreadable() const
        { return true; }
    virtual size_t used() const = 0;
    virtual size_t optgettable() const
        { return used(); }
    virtual const void *get(size_t count) = 0;
    virtual void skip(size_t count)
        { get(count); }
    virtual void unget(size_t count) = 0;
    virtual size_t ungettable() const = 0;
    virtual size_t peekable(int offset) const;
    virtual size_t optpeekable(int offset) const
        { return peekable(offset); }
    virtual const void *peek(int offset, size_t count)
        { return mutablepeek(offset, count); }
    virtual void zap() = 0;
    
    // helpers
    void move(void *buf, size_t count);
    void copy(void *buf, int offset, size_t count);
    
    /*** Buffer Writing ***/
    
    virtual bool iswritable() const
        { return true; }
    virtual size_t free() const = 0;
    virtual size_t optallocable() const
        { return free(); }
    virtual void *alloc(size_t count) = 0;
    virtual void unalloc(size_t count) = 0;
    virtual size_t unallocable() const = 0;
    virtual void *mutablepeek(int offset, size_t count) = 0;
    
    // helpers
    void put(const void *data, size_t count);
    void fastput(const void *data, size_t count);
    void poke(const void *data, int offset, size_t count);

    /*** Buffer to Buffer Transfers ***/

    virtual void merge(WvBufferStore &instore, size_t count);

    // default implementation
    void basicmerge(WvBufferStore &instore, size_t count);

    /*** Support for buffers with subbuffers ***/

    /**
     * Returns true if the buffer uses subbuffers for storage.
     */
    virtual bool usessubbuffers() const
        { return false; }

    /**
     * Returns the number of subbuffers in the buffer.
     */
    virtual size_t numsubbuffers() const
        { return 0; }

    /**
     * Returns the first subbuffer.
     * @return the buffer or NULL if none or not supported
     */
    virtual WvBufferStore *firstsubbuffer() const
        { return NULL; }

    /**
     * Appends a subbuffer to the buffer.
     */
    virtual void appendsubbuffer(WvBufferStore *buffer, bool autofree)
        { assert(! "not supported"); }

    /**
     * Prepends a subbuffer to the buffer.
     */
    virtual void prependsubbuffer(WvBufferStore *buffer, bool autofree)
        { assert(! "not supported"); }

    /**
     * Unlinks the specified subbuffer.
     * Only autofrees the buffer if allowautofree == true.
     * @return the autofree flag for the buffer
     */
    virtual bool unlinksubbuffer(WvBufferStore *buffer,
        bool allowautofree)
        { assert(! "not supported"); return true; }
};

// lists of buffer stores are sometimes useful
DeclareWvList(WvBufferStore);



/**
 * A statically bound mixin template for buffer implementations that are
 * read-only.  It is an error to attempt to write to a read-only buffer.
 * Note that read-only in this context does not mean the same as "const".
 */
template<class Super>
class WvReadOnlyBufferStoreMixin : public Super
{
public:
    explicit inline WvReadOnlyBufferStoreMixin(int _granularity) :
        Super(_granularity) { }
    virtual bool iswritable() const
    {
        return false;
    }
    virtual size_t free() const
    {
        return 0;
    }
    virtual size_t optallocable() const
    {
        return 0;
    }
    virtual void *alloc(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero alloc() called on non-writable buffer");
        return NULL;
    }
    virtual void unalloc(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero unalloc() called on non-writable buffer");
    }
    virtual size_t unallocable() const
    {
        return 0;
    }
    virtual void *mutablepeek(int offset, size_t count)
    {
        assert(count == 0 ||
            ! "mutablepeek() called on non-writable buffer");
        return NULL;
    }
    virtual void merge(WvBufferStore &instore, size_t count)
    {
        assert(count == 0 ||
            ! "non-zero merge() called on non-writable buffer");
    }
};



/**
 * A statically bound mixin template for buffer implementations that are
 * write-only.  It is an error to attempt to read from a write-only buffer.
 */
template<class Super>
class WvWriteOnlyBufferStoreMixin : public Super
{
public:
    explicit inline WvWriteOnlyBufferStoreMixin(int _granularity) :
        Super(_granularity) { }
    virtual bool isreadable() const
    {
        return false;
    }
    virtual size_t used() const
    {
        return 0;
    }
    virtual size_t optgettable() const
    {
        return 0;
    }
    virtual size_t peekable(int offset) const
    {
        return 0;
    }
    virtual size_t optpeekable(int offset) const
    {
        return 0;
    }
    virtual const void *get(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero get() called on non-readable buffer");
        return NULL;
    }
    virtual void skip(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero skip() called on non-readable buffer");
    }
    virtual void unget(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero unget() called on non-readable buffer");
    }
    virtual size_t ungettable() const
    {
        return 0;
    }
    virtual const void *peek(int offset, size_t count)
    {
        assert(count == 0 ||
            ! "peek() called on non-readable buffer");
        return NULL;
    }
    virtual void zap()
    {
        // nothing to zap
    }
};



/**
 * The WvInPlaceBuffer storage class.
 */
class WvInPlaceBufferStore : public WvBufferStore
{
protected:
    void *data;
    size_t xsize;
    size_t readidx;
    size_t writeidx;
    bool xautofree;

public:
    WvInPlaceBufferStore(int _granularity,
        void *_data, size_t _avail, size_t _size, bool _autofree);
    WvInPlaceBufferStore(int _granularity, size_t _size);
    virtual ~WvInPlaceBufferStore();
    inline void *ptr() const
        { return data; }
    inline size_t size() const
        { return xsize; }
    inline bool autofree() const
        { return xautofree; }
    inline void setautofree(bool _autofree)
        { xautofree = _autofree; }
    void reset(void *_data, size_t _avail, size_t _size, bool _autofree);
    void setavail(size_t _avail);
    
    /*** Overridden Members ***/
    virtual size_t used() const;
    virtual const void *get(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual void zap();
    virtual size_t free() const;
    virtual void *alloc(size_t count);
    virtual void unalloc(size_t count);
    virtual size_t unallocable() const;
    virtual void *mutablepeek(int offset, size_t count);
};



/**
 * The WvConstInPlaceBuffer storage class.
 */
class WvConstInPlaceBufferStore :
    public WvReadOnlyBufferStoreMixin<WvBufferStore>
{
protected:
    const void *data;
    size_t avail;
    size_t readidx;

public:
    WvConstInPlaceBufferStore(int _granularity,
        const void *_data, size_t _avail);
    inline const void *ptr() const
        { return data; }
    void reset(const void *_data, size_t _avail);
    void setavail(size_t _avail);

    /*** Overridden Members ***/
    virtual size_t used() const;
    virtual const void *get(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual const void *peek(int offset, size_t count);
    virtual void zap();
};



/**
 * The WvCircularBuffer storage class.
 */
class WvCircularBufferStore : public WvBufferStore
{
protected:
    void *data;
    size_t xsize;
    size_t head;
    size_t totalused;
    size_t totalinit;
    bool xautofree;

public:
    WvCircularBufferStore(int _granularity,
        void *_data, size_t _avail, size_t _size, bool _autofree);
    WvCircularBufferStore(int _granularity, size_t _size);
    virtual ~WvCircularBufferStore();
    inline void *ptr() const
        { return data; }
    inline size_t size() const
        { return xsize; }
    inline bool autofree() const
        { return xautofree; }
    inline void setautofree(bool _autofree)
        { xautofree = _autofree; }
    void reset(void *_data, size_t _avail, size_t _size, bool _autofree);
    void setavail(size_t _avail);
    void normalize();
    
    /*** Overridden Members ***/
    virtual size_t used() const;
    virtual size_t optgettable() const;
    virtual const void *get(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual void zap();
    virtual size_t free() const;
    virtual size_t optallocable() const;
    virtual void *alloc(size_t count);
    virtual void unalloc(size_t count);
    virtual size_t unallocable() const;
    virtual void *mutablepeek(int offset, size_t count);

protected:
    /**
     * Ensures that count new bytes can be read from or written
     * to the buffer beginning at the specified offset as one
     * large contiguous block.
     *
     * @param offset the offset
     * @param count the number of bytes
     * @param keephistory if true, does not purge unget history
     * @return the offset of the first available byte
     */
    size_t ensurecontiguous(int offset, size_t count, bool keephistory);

    /**
     * Compacts an array arranged as a circular buffer such that
     * the specified region is moved to the beginning of the array.
     *
     * @param data the array base
     * @param size the size of the array
     * @param head the beginning of the region to keep
     * @param count the number of bytes in the region to keep
     */
    static void compact(void *data, size_t size,
        size_t head, size_t count);
};



/**
 * The WvLinkedBuffer storage class.
 *
 * A buffer store built out of a list of other buffers linked together.
 * Buffers may be appended or prepended to the list at any time, at
 * which point they act as slaves for the master buffer.  Slaves may
 * be expunged from the list at any time when the master buffer
 * determines that they are of no further use.
 *
 * This is mostly useful for building other buffer storage classes.
 */
class WvLinkedBufferStore : public WvBufferStore
{
protected:
    WvBufferStoreList list;
    size_t totalused;
    size_t maxungettable;

public:
    explicit WvLinkedBufferStore(int _granularity);

    /*** Overridden Members ***/
    virtual size_t used() const;
    virtual size_t optgettable() const;
    virtual const void *get(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual void zap();
    virtual size_t free() const;
    virtual size_t optallocable() const;
    virtual void *alloc(size_t count);
    virtual void unalloc(size_t count);
    virtual size_t unallocable() const;
    virtual size_t optpeekable(int offset) const;
    virtual void *mutablepeek(int offset, size_t count);

    virtual bool usessubbuffers() const;
    virtual size_t numsubbuffers() const;
    virtual WvBufferStore *firstsubbuffer() const;
    virtual void appendsubbuffer(WvBufferStore *buffer, bool autofree);
    virtual void prependsubbuffer(WvBufferStore *buffer, bool autofree);
    virtual bool unlinksubbuffer(WvBufferStore *buffer,
        bool allowautofree);

protected:
    /**
     * Called when a new buffer must be allocated to coalesce chunks.
     *   minsize : the minimum size for the new buffer
     */
    virtual WvBufferStore *newbuffer(size_t minsize);

    /**
     * Called when a buffer with autofree is removed from the list.
     * This function is not called during object destruction.
     */
    virtual void recyclebuffer(WvBufferStore *buffer);

    /**
     * Searches for the buffer containing the offset.
     * 
     * @param it the iterator updated to point to buffer found,
     *        or to an invalid region if the offset is invalid
     * @param offset the offset for which to search
     * @return the corrected offset within the buffer at it.ptr()
     */
    int search(WvBufferStoreList::Iter &it, int offset) const;

    /**
     * Coalesces a sequence of buffers.
     *
     * @param it the iterator pointing to the first buffer
     * @param count the required number of contiguous used bytes
     * @return the composite buffer
     */
    WvBufferStore *coalesce(WvBufferStoreList::Iter &it,
        size_t count);

private:
    // unlinks and recycles the buffer pointed at by the iterator
    void do_xunlink(WvBufferStoreList::Iter &it);
};



/**
 * The WvDynamicBuffer storage class.
 */
class WvDynamicBufferStore : public WvLinkedBufferStore
{
    size_t minalloc;
    size_t maxalloc;
    
public:
    WvDynamicBufferStore(size_t _granularity,
        size_t _minalloc, size_t _maxalloc);

    /*** Overridden Members ***/
    virtual size_t free() const;
    virtual size_t optallocable() const;
    virtual void *alloc(size_t count);

protected:
    virtual WvBufferStore *newbuffer(size_t minsize);
};



/**
 * The WvEmptyBuffer storage class.
 */
class WvEmptyBufferStore : public WvWriteOnlyBufferStoreMixin<
    WvReadOnlyBufferStoreMixin<WvBufferStore> >
{
public:
    explicit WvEmptyBufferStore(size_t _granularity);
};



/**
 * The WvBufferCursor storage class.
 */
class WvBufferCursorStore :
    public WvReadOnlyBufferStoreMixin<WvBufferStore>
{
protected:
    WvBufferStore *buf;
    int start;
    size_t length;
    size_t shift;

public:
    WvBufferCursorStore(size_t _granularity, WvBufferStore *_buf,
        int _start, size_t _length);

    /*** Overridden Members ***/
    virtual bool isreadable() const;
    virtual size_t used() const;
    virtual size_t optgettable() const;
    virtual const void *get(size_t count);
    virtual void skip(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual size_t peekable(int offset) const;
    virtual size_t optpeekable(int offset) const;
    virtual const void *peek(int offset, size_t count);
    virtual void zap();
    virtual bool iswritable() const;
    virtual void *mutablepeek(int offset, size_t count);
};

#endif // __WVBUFFERSTORE_H
