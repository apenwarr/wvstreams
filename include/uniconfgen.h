/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */

/** /file
 * A UniConf key management abstraction.
 */
#ifndef __UNICONFGEN_H
#define __UNICONFGEN_H
 
#include "uniconfdefs.h"
#include "uniconflocation.h"
#include "uniconfkey.h"
#include "uniconfiter.h"
#include "wvstring.h"
#include "wvhashtable.h"
#include "wvlog.h"

class WvStreamList;

/**
 * An abstract data container that backs a UniConf tree.
 * <p>
 * This is intended to be implemented to provide support for fetching
 * and storing keys and values using different access methods.
 * </p>
 */
class UniConfGen
{
protected:
    /**
     * Creates a UniConfGen object.
     * @param location the location
     */
    UniConfGen();

public:
    /**
     * Destroys the UniConfGen and may discard uncommitted data.
     */
    virtual ~UniConfGen();
    
    /**
     * Returns the location managed by this UniConfGen.
     * @return the location
     */
    virtual UniConfLocation location() const = 0;
        
    /**
     * Commits information about a key recursively.
     * <p>
     * The default implementation always returns true.
     * </p>
     * @param key the key
     * @param depth the recursion depth
     * @return true on success
     * @see UniConfDepth::Type
     */
    virtual bool commit(const UniConfKey &key,
        UniConfDepth::Type depth);
    
    /**
     * Refreshes information about a key recursively.
     * <p>
     * May discard uncommitted data.
     * </p><p>
     * The default implementation always returns true.
     * </p>
     * @param key the key
     * @param depth the recursion depth
     * @return true on success
     * @see UniConfDepth::Type
     */
    virtual bool refresh(const UniConfKey &key,
        UniConfDepth::Type depth);

    /**
     * Fetches a string value for a key from the registry.
     * @param key the key
     * @return the value, or WvString::null if the key does not exist
     */
    virtual WvString get(const UniConfKey &key) = 0;
    
    /**
     * Stores a string value for a key into the registry.
     * @param key the key
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    virtual bool set(const UniConfKey &key, WvStringParm value) = 0;

    /**
     * Removes a key and all of its children from the registry.
     * <p>
     * Non-virtual synonym for set(key, WvString::null).
     * </p>
     * @param key the key
     * @return true on success
     */
    bool remove(const UniConfKey &key);

    /**
     * Removes the children of a key from the registry.
     * @param key the key
     * @return true on success
     */
    virtual bool zap(const UniConfKey &key) = 0;

    /**
     * Returns true if a key exists without fetching its value.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     * </p><p>
     * The default implementation returns ! get(key).isnull().
     * </p>
     * @param key the key
     * @return true if the key exists
     */
    virtual bool exists(const UniConfKey &key);

    /**
     * Returns true if a key has children.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     * </p>
     * @param key the key
     * @return true if the key has children
     */
    virtual bool haschildren(const UniConfKey &key) = 0;

    /**
     * Determines if the generator is usable.
     * <p>
     * The default implementation always returns true.
     * </p>
     * @return true if the generator is okay
     */
    virtual bool isok();

    /**
     * Gives UniConf an opportunity to append streams to a streamlist.
     * <p>
     * This method must be called at most once until the next detach().
     * </p><p>
     * The default implementation does nothing.
     * </p>
     * @param streamlist the stream list, non-NULL
     * @see detach(WvStreamList*)
     */
    virtual void attach(WvStreamList *streamlist);

    /**
     * Gives UniConf an opportunity to unlink streams from a streamlist.
     * <p>
     * This method must be called exactly once for each attach().
     * </p>
     * @param streamlist the stream list, non-NULL
     * @see attach(WvStreamList*)
     */
    virtual void detach(WvStreamList *streamlist);

    /**
     * Base type for all UniConfGen iterators.
     */
    typedef UniConfAbstractIter Iter;

    /**
     * A null iterator type implementation.
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

    /**
     * Returns an iterator over the children of the specified key.
     * @param key the key
     * @return the iterator instance
     */
    virtual Iter *iterator(const UniConfKey &key) = 0;
};



/**
 * A UniConfGen that delegates requests to an inner generator.
 */
class UniConfFilterGen : public UniConfGen
{
    UniConfGen *xinner;

protected:
    UniConfFilterGen(UniConfGen *inner);
    virtual ~UniConfFilterGen();

public:
    inline UniConfGen *inner() const
        { return xinner; }

    /***** Overridden methods *****/

    virtual UniConfLocation location() const;
    virtual bool commit(const UniConfKey &key,
        UniConfDepth::Type depth);
    virtual bool refresh(const UniConfKey &key,
        UniConfDepth::Type depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual void attach(WvStreamList *streamlist);
    virtual void detach(WvStreamList *streamlist);
    virtual bool isok();
    virtual Iter *iterator(const UniConfKey &key);
};


/**
 * A UniConfGen factory.
 */
class UniConfGenFactory
{
protected:
    UniConfGenFactory();
    
public:
    /**
     * Destroys the factory.
     */
    virtual ~UniConfGenFactory();

    /**
     * Returns a new UniConfGen instance for the specified location.
     * @param location the location
     * @return the instance, or NULL on failure
     */
    virtual UniConfGen *newgen(const UniConfLocation &location) = 0;
};



/**
 * This class simplifies registering factories during module
 * initialization and unregistering them at shutdown.
 * <p>
 * Example: To register a factory call AFactory for protocol "a"...
 * <pre>
 * static UniConfGenFactoryRegistration areg("a", new AFactory());
 * </pre>
 * </p>
 */
class UniConfGenFactoryRegistration
{
public:
    WvString proto;
    UniConfGenFactory *factory;
    bool autofree;

    /**
     * Automatically registers the factory with the registry.
     * @param proto the protocol
     * @param factory the factory
     * @param autofree if true, takes ownership of the factory
     */
    UniConfGenFactoryRegistration(WvStringParm _proto,
        UniConfGenFactory *_factory, bool _autofree);
        
    /**
     * Automatically unregisters the factory from the registry.
     * The factory is also deleted if autofree == true.
     */
    ~UniConfGenFactoryRegistration();
};
DeclareWvDict(UniConfGenFactoryRegistration, WvString, proto);



/**
 * A registry of UniConfGen factories.
 * This object is a singleton.
 */
class UniConfGenFactoryRegistry : public UniConfGenFactory
{
    static UniConfGenFactoryRegistry *singleton;
    UniConfGenFactoryRegistrationDict dict;
    WvLog log;

protected:
    /**
     * Creates an empty registry.
     */
    UniConfGenFactoryRegistry();

public:    
    /**
     * Deletes all factories registered with autofree == true.
     */
    virtual ~UniConfGenFactoryRegistry();

    /**
     * Returns the singleton instance of the registry.
     * @return the instance
     */
    static UniConfGenFactoryRegistry *instance();

    /**
     * Registers a factory.
     * @param registration the registration
     */
    void add(UniConfGenFactoryRegistration *reg);

    /**
     * Unregisters a factory.
     * @param registration the registration
     */
    void remove(UniConfGenFactoryRegistration *reg);

    /**
     * Returns the factory for the specified protocol.
     *
     * @param proto the protocol
     * @return the factory, or NULL if none has been registered
     */
    UniConfGenFactory *find(WvStringParm proto);

    /***** Overridden methods *****/

    virtual UniConfGen *newgen(const UniConfLocation &location);
};


#endif // UNICONFGEN_H
