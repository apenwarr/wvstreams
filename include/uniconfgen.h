/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#ifndef __UNICONFGEN_H
#define __UNICONFGEN_H
 
#include "uniconfdefs.h"
#include "uniconfkey.h"
#include "uniconfiter.h"
#include "wvcallback.h"
#include "wvxplc.h"

class WvStreamList;
class UniConfGen;

DeclareWvCallback(4, void, UniConfGenCallback, const UniConfGen &,
    const UniConfKey &, UniConfDepth::Type, void *);

/**
 * An abstract data container that backs a UniConf tree.
 *
 * This is intended to be implemented to provide support for fetching
 * and storing keys and values using different access methods.
 *
 */
class UniConfGen : public GenericComponent<IObject>
{
protected:
    UniConfGenCallback cb;
    void *cbdata;

    /** Creates a UniConfGen object. */
    UniConfGen();

    /**
     * Sends notification that a key has changed value.
     */
    void delta(const UniConfKey &key, UniConfDepth::Type depth);

    /**
     * Raises an error condition.
     */
    void seterror(WvStringParm error)
        { } // FIXME: decide on final API for this probably WvError

public:
    /** Destroys the UniConfGen and may discard uncommitted data. */
    virtual ~UniConfGen();
    
    /**
     * Commits information about a key recursively.
     * 'depth' is the recursion depth.  Returns true on success.
     *
     * The default implementation always just returns true.
     * 
     * @see UniConfDepth::Type
     */
    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth);
    
    /**
     * Refreshes information about a key recursively.
     * May discard uncommitted data.
     *
     * The default implementation always returns true.
     *
     * @see UniConfDepth::Type
     */
    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);

    /**
     * Fetches a string value for a key from the registry.  If the key doesn't
     * exist, the return value is WvString::null.
     */
    virtual WvString get(const UniConfKey &key) = 0;
    
    /**
     * Stores a string value for a key into the registry.  If the value is
     * WvString::null, the key is deleted.
     * 
     * Returns true on success.
     */
    virtual bool set(const UniConfKey &key, WvStringParm value) = 0;

    /**
     * Removes a key and all of its children from the registry.  Returns
     * true on success.
     *
     * Non-virtual synonym for set(key, WvString::null).
     */
    bool remove(const UniConfKey &key);

    /**
     * Removes all children of a key from the registry. Returns true on
     * success.
     */
    virtual bool zap(const UniConfKey &key) = 0;

    /**
     * Without fetching its value, returns true if a key exists.
     *
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     *
     * The default implementation returns !get(key).isnull().
     */
    virtual bool exists(const UniConfKey &key);

    /**
     * Returns true if a key has children.
     *
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     */
    virtual bool haschildren(const UniConfKey &key) = 0;

    /**
     * Determines if the generator is usable and working properly.
     *
     * The default implementation always returns true.
     */
    virtual bool isok();

    /**
     * Sets the callback for change notification.
     */
    void setcallback(const UniConfGenCallback &callback, void *userdata);
    
    /** Base type for all UniConfGen iterators. */
    typedef UniConfAbstractIter Iter;

    /**
     * An iterator that's always empty.  This is handy if you don't have
     * anything good to iterate over.
     */
    class NullIter : public Iter
    {
    public:
        NullIter()
            { }
        NullIter(const NullIter &other) : Iter(other)
            { }
    
        virtual Iter *clone() const
            { return new NullIter(*this); }
        virtual void rewind()
            { }
        virtual bool next()
            { return false; }
        virtual UniConfKey key() const
            { return UniConfKey::EMPTY; }
    };

    /** Returns an iterator over the children of the specified key. */
    virtual Iter *iterator(const UniConfKey &key) = 0;
};

DEFINE_IID(UniConfGen, {0x7ca76e98, 0xb694, 0x43ca,
    {0xb0, 0x56, 0x8b, 0x9d, 0xde, 0x9a, 0xbe, 0x9f}});



/**
 * A UniConfGen that delegates all requests to an inner generator.  If you
 * derive from this, you can selectively override particular behaviours
 * of a sub-generator.
 */
class UniConfFilterGen : public UniConfGen
{
    UniConfGen *xinner;

protected:
    UniConfFilterGen(UniConfGen *inner);
    virtual ~UniConfFilterGen();

    /**
     * Rebinds the inner generator.
     */
    void setinner(UniConfGen *inner);

public:
    inline UniConfGen *inner() const
        { return xinner; }

    /***** Overridden methods *****/

    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth);
    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual bool isok();
    virtual Iter *iterator(const UniConfKey &key);

protected:
    /**
     * Called by inner generator when a key changes.
     * The default implementation calls delta(key, depth).
     */
    virtual void gencallback(const UniConfGen &gen, const UniConfKey &key,
        UniConfDepth::Type depth, void *userdata);
};

#endif // UNICONFGEN_H
