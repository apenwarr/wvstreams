/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Several kinds of UniConf iterators.
 */
#ifndef __UNICONFITER_H
#define __UNICONFITER_H

#include "uniconfkey.h"
#include "uniconfdefs.h"

/**
 * An abstract iterator over keys and values.
 *
 * Unlike other WvStreams iterators, this one declares virtual
 * methods so that the various UniConf components can supply
 * the right behaviour through a common interface that does not
 * depend on static typing.
 *
 * The precise traversal sequence is defined by the iterator
 * implementation.  For instance, the same interface is used for
 * flat, recursive and filtered queries but the instantiation method
 * may differ for each.
 *
 * FIXME: Should UniConf support concurrent modifications??
 *
 */
class UniConfAbstractIter
{
protected:
    UniConfAbstractIter()
        { }
    UniConfAbstractIter(const UniConfAbstractIter &iter) 
        { }

public:
    /** Destroys the iterator. */
    virtual ~UniConfAbstractIter()
        { }

#if 0
//FIXME: not sure if this was really a good idea...
    /** Clones the iterator. */
    virtual UniConfAbstractIter *clone() const = 0;
#endif

    /** Rewinds the iterator. */
    virtual void rewind() = 0;

    /**
     * Seeks to the next element in the sequence, and returns true if that
     * element exists.
     */
    virtual bool next() = 0;

    /** Returns the current key. */
    virtual UniConfKey key() const = 0;

#if 0
    /** Returns the current value as a string. */
    virtual WvString get() const = 0;

    /**
     * Sets the current value as a string.
     *
     * If the key is deleted ('value' is null), the iteration sequence
     * resumes at the next available element without repeating any that
     * were previously encountered.
     */
    virtual bool set(WvString value) = 0;

    /** Removes the current key without advancing the iterator. */
    bool remove()
        { return set(WvString::null); }
#endif
};


/**
 * This wrapper manages the lifetime of an abstract iterator and provides
 * a base upon which subclasses may be constructed that provide the more
 * familiar statically typed WvStream iterator interface.
 */
class UniConfIterWrapper
{
protected:
    UniConfAbstractIter *xabstractiter;

public:
    /** Constructs a wrapper from any other iterator. */
    UniConfIterWrapper(UniConfAbstractIter *abstractiter)
        : xabstractiter(abstractiter)
        { }
    
#if 0
    /**
     * Constructs a copy of another wrapper.
     * "other" is the other wrapper
     */
    UniConfIterWrapper(const UniConfIterWrapper &other) 
        : xabstractiter(other.abstractiter()->clone())
        { }
#endif

    /** Destroys the wrapper along with its abstract iterator. */
    ~UniConfIterWrapper()
        { delete xabstractiter; }

    /** Returns a pointer to the abstract iterator. */
    UniConfAbstractIter *abstractiter() const
        { return xabstractiter; }

    void rewind()
        { xabstractiter->rewind(); }

    bool next()
        { return xabstractiter->next(); }
    
    UniConfKey key() const
        { return xabstractiter->key(); }
};

#endif // __UNICONFITER_H
