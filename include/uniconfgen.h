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
#include "wvcallback.h"
#include "wvxplc.h"
#include "wvlinklist.h"

class UniConfGen;

/**
 * The callback type for signalling key changes from a UniConfGen.
 * 
 * Generators that wrap other generators should catch notifications
 * and reissue them using themselves as the "gen" parameter and their
 * userdata as the "userdata parameter".  This can be done effectively by
 * invoking the delta() function on receipt of a notification from a
 * wrapped generator.  See UniFilterGen.
 * 
 * Parameters: gen, key, userdata
 * 
 * gen - the externally visible generator whose key has changed
 * key - the key that has changed
 * userdata - the userdata supplied during setcallback
 */
DeclareWvCallback(3, void, UniConfGenCallback, UniConfGen *,
    const UniConfKey &, void *);

/**
 * An abstract data container that backs a UniConf tree.
 *
 * This is intended to be implemented to provide support for fetching
 * and storing keys and values using different access methods.
 *
 */
class UniConfGen : public GenericComponent<IObject>
{
    // These fields are deliberately hidden to encourage use of the
    // delta() member in case the notification mechanism changes.
    UniConfGenCallback cb; //!< gets called whenever a key changes its value.
    void *cbdata;
    
protected:
    /** Creates a UniConfGen object. */
    UniConfGen();

    /**
     * Sends notification that a key has possibly changed.
     * This takes care of the details of invoking the callback.
     */
    void delta(const UniConfKey &key);

    /** Raises an error condition. */
    void seterror(WvStringParm error)
        { } // FIXME: decide on final API for this probably WvError

public:
    /** Destroys the UniConfGen and may discard uncommitted data. */
    virtual ~UniConfGen();
    
    /**
     * Sets the callback for change notification.
     * Must not be reimplemented by subclasses.
     */
    void setcallback(const UniConfGenCallback &callback, void *userdata);
    
    /**
     * Commits information about a key recursively.
     * 'depth' is the recursion depth.  Returns true on success.
     *
     * The default implementation always just returns true.
     */
    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth);
    
    /**
     * Refreshes information about a key recursively.
     * May discard uncommitted data.
     *
     * The default implementation always returns true.
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
     * Removes all children of a key from the registry. Returns true on
     * success.
     *
     * The default implementation iterates over all children using
     * iterator() removing them using set(key, WvString::null).
     * Subclasses are strongly encouraged to provide a better implementation.
     */
    virtual bool zap(const UniConfKey &key);

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
     * 
     * The default implementation uses the iterator returned by iterator()
     * to test whether the child has any keys.
     * Subclasses are strongly encouraged to provide a better implementation.
     */
    virtual bool haschildren(const UniConfKey &key);

    /**
     * Determines if the generator is usable and working properly.
     *
     * The default implementation always returns true.
     */
    virtual bool isok();

    /** The abstract iterator type (see below) */
    class Iter;

    /** A concrete null iterator type (see below) */
    class NullIter;

    /**
     * Returns an iterator over the children of the specified key.
     * Must not return NULL; consider returning a NullIter instead.
     *
     * The caller takes ownership of the returned iterator and is responsible
     * for deleting it when finished.
     */
    virtual Iter *iterator(const UniConfKey &key) = 0;
};

DEFINE_IID(UniConfGen, {0x7ca76e98, 0xb694, 0x43ca,
    {0xb0, 0x56, 0x8b, 0x9d, 0xde, 0x9a, 0xbe, 0x9f}});
DeclareWvList(UniConfGen);


/**
 * An abstract iterator over keys and values in a generator.
 *
 * Unlike other WvStreams iterators, this one declares virtual methods so
 * that UniConfGen implementations can supply the right behaviour
 * through a common interface that does not depend on static typing.
 *
 * The precise traversal sequence is defined by the iterator implementation.
 *
 * The iterator need not support concurrent modifications of the underlying
 * data structures.
 * 
 * TODO: Consider changing this rule depending on observed usage patterns.
 */
class UniConfGen::Iter
{
public:
    /** Destroys the iterator. */
    virtual ~Iter() { }

    /**
     * Rewinds the iterator.
     * Must be called prior to the first invocation of next().
     */
    virtual void rewind() = 0;

    /**
     * Seeks to the next element in the sequence.
     * Returns true if that element exists.
     * Must be called prior to the first invocation of key().
     */
    virtual bool next() = 0;

    /** Returns the current key. */
    virtual UniConfKey key() const = 0;
};


/**
 * An iterator that's always empty.
 * This is handy if you don't have anything good to iterate over.
 */
class UniConfGen::NullIter : public UniConfGen::Iter
{
public:
    /***** Overridden members *****/
    
    virtual void rewind() { }
    virtual bool next() { return false; }
    virtual UniConfKey key() const { return UniConfKey::EMPTY; }
};

#endif // UNICONFGEN_H
