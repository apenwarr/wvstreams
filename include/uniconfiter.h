/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Several kinds of UniConf iterators.
 */
#ifndef __UNICONFITER_H
#define __UNICONFITER_H

#include "uniconfkey.h"

/**
 * An abstract iterator over keys and values.
 * <p>
 * Unlike other WvStreams iterators, this one declares virtual
 * methods so that the various UniConf components can supply
 * the right behaviour through a common interface that does not
 * depend on static typing.
 * </p><p>
 * The precise traversal sequence is defined by the iterator
 * implementation.  For instance, the same interface is used for
 * flat, recursive and filtered queries but the instantiation method
 * may differ for each.
 * </p><p>
 * FIXME: Should UniConf support concurrent modifications??
 * </p>
 */
class UniConfAbstractIter
{
protected:
    UniConfAbstractIter() { }
    UniConfAbstractIter(const UniConfAbstractIter &iter) { }

public:
    /**
     * Destroys the iterator.
     */
    virtual ~UniConfAbstractIter() { }

    /**
     * Clones the iterator.
     */
    virtual UniConfAbstractIter *clone() const = 0;

    /**
     * Rewinds the iterator.
     */
    virtual void rewind() = 0;

    /**
     * Seeks to the next element in the controlled sequence.
     * @return true if there is another element
     */
    virtual bool next() = 0;

    /**
     * Returns the current key.
     * @return the key
     */
    virtual UniConfKey key() const = 0;

#if 0
    /**
     * Returns the current value as a string.
     * @return the value
     */
    virtual WvString get() const = 0;

    /**
     * Sets the current value as a string.
     * <p>
     * If the key is deleted, the iteration sequence resumes at
     * the next available element without repeating any that
     * were previously encountered.
     * </p>
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    virtual bool set(WvString value) = 0;

    /**
     * Removes the current key without advancing the iterator.
     * @return true on success
     */
    bool remove()
    {
        return set(WvString::null);
    }
#endif
};


/**
 * This wrapper manages the lifetime of an abstract iterator
 * and provides a base upon which subclasses may be constructed
 * that provide a the more familiar statically typed WvStream
 * iterator interface.
 */
class UniConfIterWrapper
{
protected:
    UniConfAbstractIter *xabstractiter;

public:
    /**
     * Constructs a wrapper from an abstract iterator.
     * @param iter the iterator
     */
    UniConfIterWrapper(UniConfAbstractIter *abstractiter) :
        xabstractiter(abstractiter)
    {
    }
    
    /**
     * Constructs a copy of another wrapper.
     * @param other the other wrapper
     */
    UniConfIterWrapper(const UniConfIterWrapper &other) :
        xabstractiter(other.abstractiter()->clone())
    {
    }

    /**
     * Destroys the wrapper along with its abstract iterator.
     */
    ~UniConfIterWrapper()
    {
        delete xabstractiter;
    }

    /**
     * Returns a pointer to the abstract iterator.
     * @return the abstract iterator
     */
    inline UniConfAbstractIter *abstractiter() const
    {
        return xabstractiter;
    }

    inline void rewind()
    {
        xabstractiter->rewind();
    }

    inline bool next()
    {
        return xabstractiter->next();
    }
    
    inline UniConfKey key() const
    {
        return xabstractiter->key();
    }
};

#endif // __UNICONFITER_H
