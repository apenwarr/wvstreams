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

// This value is used internally to signal unlimited free space.
// It is merely meant to be as large as possible yet leave enough
// room to accomodate simple arithmetic operations without overflow.
// Clients should NOT check for the presence of this value explicitly.
#define UNLIMITED_FREE_SPACE (INT_MAX/2)

//#define DYNAMIC_BUFFER_POOLING_EXPERIMENTAL

/**
 * The abstract buffer storage base class.
 */
class WvBufferStore
{
private:
    // discourage copying
    WvBufferStore(const WvBufferStore &other) { }

protected:
    // the suggested granularity
    int granularity;

    /**
     * Creates a new buffer.
     * @param _granularity the suggested granularity for data allocation
     *        and alignment purposes
     */
    WvBufferStore(int _granularity);
    
public:
    virtual ~WvBufferStore() { }

    /*** Buffer Reading ***/
    
    virtual bool isreadable() const
        { return true; }
    virtual size_t used() const = 0;
    virtual size_t optgettable() const
        { return used(); }
    virtual const void *get(size_t count) = 0;
    virtual void unget(size_t count) = 0;
    virtual size_t ungettable() const = 0;
    virtual const void *peek(int offset, size_t *count, size_t mincount)
        { return mutablepeek(offset, count, mincount); }
    virtual void zap() = 0;
    
    // helpers
    void move(void *buf, size_t count);
    void copy(void *buf, size_t count, int offset);
    
    /*** Buffer Writing ***/
    
    virtual bool iswritable() const
        { return true; }
    virtual size_t free() const = 0;
    virtual size_t optallocable() const
        { return free(); }
    virtual void *alloc(size_t count) = 0;
    virtual void unalloc(size_t count) = 0;
    virtual size_t unallocable() const = 0;
    virtual void *mutablepeek(int offset, size_t *count,
        size_t mincount) = 0;
    
    // helpers
    void put(const void *data, size_t count);
    void fastput(const void *data, size_t count);
    void poke(const void *data, size_t count, int offset);

    /*** Buffer to Buffer Transfers ***/
    
    virtual void merge(WvBufferStore &instore, size_t count)
        { basicmerge(instore, count); }
    virtual void mergeall(WvBufferStore &instore, size_t count)
        { merge(instore, instore.used()); }

    // default implementation
    void basicmerge(WvBufferStore &instore, size_t count);
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
    inline WvReadOnlyBufferStoreMixin(int _granularity) :
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
    virtual void *mutablepeek(int offset, size_t *count, size_t mincount)
    {
        assert(! "mutablepeek() called on non-writable buffer");
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
    inline WvWriteOnlyBufferStoreMixin(int _granularity) :
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
    virtual const void *get(size_t count)
    {
        assert(count == 0 ||
            ! "non-zero get() called on non-readable buffer");
        return NULL;
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
    virtual const void *peek(int offset, size_t *count, size_t mincount)
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
    virtual void *mutablepeek(int offset, size_t *count, size_t mincount);
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
    virtual const void *peek(int offset, size_t *count, size_t mincount);
    virtual void zap();
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

public:
    WvLinkedBufferStore(int _granularity);
    // Appends a buffer store to the list.
    void append(WvBufferStore *buffer, bool autofree);
    // Prepends a buffer store to the list.
    void prepend(WvBufferStore *buffer, bool autofree);
    // Unlinks a buffer store from the list.
    void unlink(WvBufferStore *buffer);
    // Returns the number of buffer stores in the list.
    size_t numbuffers() const;

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
    virtual void *mutablepeek(int offset, size_t *count, size_t mincount);

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
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
    WvBufferStoreList pool;
#endif
    
public:
    WvDynamicBufferStore(size_t _granularity,
        size_t _minalloc, size_t _maxalloc);

    /*** Overridden Members ***/
    virtual size_t free() const;
    virtual size_t optallocable() const;
    virtual void *alloc(size_t count);

protected:
    virtual WvBufferStore *newbuffer(size_t minsize);
    
#ifdef DYNAMIC_BUFFER_POOLING_EXPERIMENTAL
    virtual void recyclebuffer(WvBufferStore *buffer);
#endif
};



/**
 * The WvEmptyBuffer storage class.
 */
class WvEmptyBufferStore : public WvWriteOnlyBufferStoreMixin<
    WvReadOnlyBufferStoreMixin<WvBufferStore> >
{
public:
    WvEmptyBufferStore(size_t _granularity);
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
    int end;
    int offset;
    WvDynamicBufferStore tmpbuf;

public:
    WvBufferCursorStore(size_t _granularity, WvBufferStore *_buf,
        int _start, size_t _length);

    /*** Overridden Members ***/
    virtual size_t used() const;
    virtual size_t optgettable() const;
    virtual const void *get(size_t count);
    virtual void unget(size_t count);
    virtual size_t ungettable() const;
    virtual const void *peek(int offset, size_t *count, size_t mincount);
    virtual void zap();
    virtual bool iswritable() const;
    virtual void *mutablepeek(int offset, size_t *count, size_t mincount);

protected:
    int getpos() const;
};

#endif // __WVBUFFERSTORE_H
