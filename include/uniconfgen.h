/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */

/** /file
 * A UniConf key management abstraction.
 */
#ifndef __UNICONFGEN_H
#define __UNICONFGEN_H
 
#include "uniconflocation.h"
#include "uniconfkey.h"
#include "wvcallback.h"
#include "wvstring.h"
#include "wvlinklist.h"
#include "wvhashtable.h"
#include "wvlog.h"

class UniConf;

/**
 * An abstract data container that backs a UniConf tree.
 * <p>
 * This is intended to be implemented to provide support for fetching
 * and storing keys and values using different access methods.
 * </p>
 *
 * FIXME: Right now used to generate UniConf objects in a tree...
 */
class UniConfGen
{
    UniConfLocation _location;
    
protected:
    /**
     * Creates a UniConfGen for the specified location.
     * @param location the location
     */
    UniConfGen(const UniConfLocation &location);

public:
    /**
     * Destroys the UniConfGen and may discard uncommitted data.
     */
    virtual ~UniConfGen();
    
    /**
     * Returns the location managed by this UniConfGen.
     * @return the location
     */
    inline UniConfLocation location() const
        { return _location; }

    // this function may return NULL if the object "shouldn't" exist
    // (in the opinion of the generator)
    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
   
    virtual void enumerate_subtrees(UniConf *conf, bool recursive);
    virtual void update(UniConf *&h);
    virtual void pre_get(UniConf *&h);
    virtual bool isok() { return true; }

    // Updates all data I am responsible for
    virtual void update_all();
    
    // the default load/save functions don't do anything... you might not
    // need them to.
    virtual void load();
    virtual void save();
};



/**
 * A UniConfGen factory.
 */
class UniConfGenFactory
{
protected:
    UniConfGenFactory();
    
public:
    virtual ~UniConfGenFactory();

    /**
     * Returns a new UniConfGen instance for the specified location.
     *
     * @return the instance, or NULL on failure
     */
    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top) = 0;
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

    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top);
};


#endif // UNICONFGEN_H
