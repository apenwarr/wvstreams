/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides an auto-resizing array data structure.
 */
#ifndef __WVVECTOR_H
#define __WVVECTOR_H

#include "wvxplc.h"
#include "wvlink.h"
#include "wvtypetraits.h"
#include <string.h>
#include <cstdio>

/**
 * The untyped vector base type.
 * @see WvVector
 */
class WvVectorBase
{
private:
    WvVectorBase(const WvVectorBase &);	// hide the copy constructor

protected:
    static const int DEFAULTSIZE = 4;
        /*!< the minimum number of slots to allocate */

    WvLink **xseq; /*!< the controlled sequence */
    int xcount; /*!< the number of elements in the sequence */
    int xslots; /*!< the capacity of the array */

    /** Creates an empty vector. */
    WvVectorBase(int slots = DEFAULTSIZE);

    /** Default WvVectorBase destructor */
    ~WvVectorBase()
    {
	if (xseq)
	    free(xseq);
    }

    /** Removes the element at the specified slot. */
    void remove(int slot);

    /** Inserts an element at the specified slot. */
    void insert(int slot, WvLink *elem);

    /** Prepend an element onto the front of the vector. */
    void prepend(WvLink *elem)
    {
	insert(0, elem);
    }

    /** Appends an element onto the tail of the vector. */
    void append(WvLink *elem);

    /** Gets autofree for the specified slot. */
    bool get_autofree(int slot)
    {
	if (slot >= 0 && slot < xcount)
	    return xseq[slot]->get_autofree();
	return false;
    }

    /** Sets autofree for the specified slot. */
    void set_autofree(int slot, bool autofree)
    {
	if (slot >= 0 && slot < xcount)
	    xseq[slot]->set_autofree(autofree);
    }

    // Comparison functions for use later in qsort()
    typedef int (*comparison_type_t)(const void *, const void *);
private:
    static comparison_type_t innercomparator;
protected:
    static int wrapcomparator(const void *_a, const void *_b)
    {
	WvLink *a = *static_cast<WvLink**>(const_cast<void*>(_a));
	WvLink *b = *static_cast<WvLink**>(const_cast<void*>(_b));
	return innercomparator(a->data, b->data);
    }

    void qsort(comparison_type_t comparator)
    {
	innercomparator = comparator;
	::qsort(xseq, xcount, sizeof(WvLink*), &WvVectorBase::wrapcomparator);
    }

public:
    class IterBase;
    friend class IterBase;

    /** Returns the number of elements actually stored in the vector. */
    int count() const
    {
	return xcount;
    }

    /** Returns true if the vector is empty. */
    bool isempty() const
    {
	return xcount == 0;
    }

    /** The number of elements that could be stored without resizing. */
    int get_capacity() const
    {
	return xslots;
    }

    /**
     * Adjusts the capacity of the vector.
     *
     * If the new capacity is greater than the old one, extends the array
     * size without actually filling in any elements.
     */
    void set_capacity(int newslots);

    /** Compacts the vector to minimize its footprint. */
    void compact()
    {
	set_capacity(xcount);
    }

    class IterBase
    {
    protected:
	const WvVectorBase &vec;
	int i;
	WvLink *link;

	friend class WvVectorBase;

    public:
	/**
	 * Binds the iterator to the specified vector.
	 */
	IterBase(const WvVectorBase &v)
	    : vec(v), i(-1), link(NULL)
	{
	}

	/**
	 * Rewinds the iterator to make it point to an imaginary
	 * element preceeding the first element of the vector.
	 */
	void rewind()
	{
	    i = -1;
	    link = (vec.xcount >= 0) ? vec.xseq[0] : NULL;
	}

	/**
	 * Unwinds the iterator to make it point to the last
	 * element of the vector.
	 */
	void unwind()
	{
	    i = vec.xcount - 1;
	    link = (i >= 0) ? vec.xseq[i] : NULL;
	}

	/**
	 * Moves the iterator along the vector to point ot the next element.
	 *
	 * If the iterator had just been rewound, it now points to the
	 * first element of the list.
	 *
	 * Returns: the current WvLink pointer, or null if there were no
	 *          more elements remaining in the traversal sequence
	 */
	WvLink *next()
	{
	    if (++i > vec.xcount - 1)
		return NULL;
	    else
	    {
		link = vec.xseq[i];
		return link;
	    }
	}

	/**
	 * Moves the iterator along the vector to point ot the next element.
	 *
	 * Returns: the current WvLink pointer, or null if there were no
	 *          more elements remaining in the traversal sequence
	 */
	WvLink *prev()
	{
	    if (--i < 0)
		return NULL;
	    else
		return vec.xseq[i];
	}

	/**
	 * Returns a pointer to the current WvLink at the iterator's current
	 * location.
	 *
	 * Returns: the current WvLink pointer, or null if there were no
	 *          more elements remain.
	 */
	WvLink *cur() const
	{
	    return link;
	}

        /**
         * Rewinds the iterator and repositions it over the element that
         * matches the specified value.
         *
         * Uses pointer equality (object identity) as the criteria for
         * finding the matching element.
         *
         * In order to locate multiple matching elements, first call find()
         * and then use find_next().
         *
         * Returns: the current WvLink pointer, or null if no such element
         *          was found
         */
	WvLink *find(const void *data);

        /**
         * Repositions the iterator over the element that matches the
         * specified value.
         *
         * Uses pointer equality (object identity) as the criteria for
         * finding the matching element.
         *
         * Returns: the current WvLink pointer, or null if no such element
         *          was found
         */
	WvLink *find_next(const void *data);
    };
};

/**
 * A dynamic array data structure with constant time lookup,
 * linear time insertion / removal, and expected logarithmic time
 * append.
 */
template<class T>
class WvVector : public WvVectorBase
{
public:
    /** Creates an empty vector. */
    WvVector()
	: WvVectorBase()
    {
    }

    /**
     * Creates an empty vector, but initialises a new number of default
     * slots.
     */
    WvVector(int slots)
	: WvVectorBase(slots)
    {
    }

    /** Destroys the vector and all of its contents. */
    ~WvVector()
    {
	zap();
    }

    /** Dereferences a particular slot of the vector. */
    T *operator[] (int slot)
    {
	if (slot >= 0 && slot < xcount)
	    return static_cast<T *>(xseq[slot]->data);
	return NULL;
    }

    /** Removes all elements from the vector. */
    void zap(bool destroy = true)
    {
	if (xcount > 0)
	    for (int i = xcount - 1; i >= 0; --i)
		remove(i, destroy);
    }

    /** Returns the first element */
    T *first()
    {
	return (*this)[0];
    }

    /** Returns the last element */
    T *last()
    {
	return (*this)[xcount - 1];
    }

    /** Removes a particular slot from the vector. */
    void remove(int slot, bool destroy = true)
    {
        WvLink *l = xseq[slot];
        T *obj = ((destroy && l->get_autofree())
		  ? static_cast<T*>(l->data)
		  : NULL);
	if (obj)
	    WvTraits<T>::release(obj);
	delete l;
	WvVectorBase::remove(slot);
    }

    /** Removes the last element */
    void remove_last(bool destroy = true)
    {
	if (xcount)
	    remove(xcount - 1, destroy);
    }

    /** Insert an element into a slot, and shifts the others to the right. */
    void insert(int slot, T *elem, bool autofree)
    {
	WvVectorBase::insert(slot, new WvLink(elem, autofree));
    }

    /** Prepends an element to the start of the vector */
    void prepend(T *elem, bool autofree)
    {
	WvVectorBase::prepend(new WvLink(elem, autofree));
    }

    /** Appends an element to the end of the vector */
    void append(T *elem, bool autofree)
    {
	WvVectorBase::append(new WvLink(elem, autofree));
    }

    /**
     * Runs the C qsort() routine over the vector, as long as the vector
     * is big enough.  It uses comparator like qsort() does to figure out
     * the order of the operators.
     */
    typedef int (*comparison_type_fn_t)(const T *, const T *);
    void qsort(comparison_type_fn_t comparator)
    {
	if (xcount < 2)
	    return;
	WvVectorBase::qsort(reinterpret_cast<comparison_type_t>(comparator));
    }

    /** A simple iterator that walks through all elements in the list. */
    class Iter : public WvVectorBase::IterBase
    {
    public:
	/** Binds the iterator to the specified vector. */
	Iter(const WvVector &v) : IterBase(v)
	{
	}

	/** Returns a pointer to the current element */
	T *ptr() const
	{
	    return static_cast<T *>(cur()->data);
	}

	WvIterStuff(T);

	/**
	 * Returns the state of autofree for the current element.
	 */
	bool get_autofree() const
	{
	    return link->get_autofree();
	}

	/**
	 * Sets the state of autofree for the current element.
	 */
	void set_autofree(bool autofree)
	{
	    link->set_autofree(autofree);
	}

        /**
         * Remove the current element from the vector and automatically
         * increments the iterator to point to the next element as if
         * next() had been called.
         */
        void remove(bool destroy = true)
        {
	    WvVector::vec.remove(i, destroy);
        }

        /**
         * Removes the current element from the vector but unlike remove()
         * automatically returns the iterator to the previous item in
         * the vector such that next() must be called to obtain the
         * next element.
         *
         * This version allows for writing neater loop structures since
         * an element can be removed in mid-traversal while still allowing
         * the iterator to be incremented at the top of the loop as usual.
         *
         * Calling xremove() twice in a row is currently unsupported.
         *
         */
	void xremove(bool destroy = true)
	{
	    WvVector::vec.remove(i, destroy);
	    prev();
	}
    };
};

#define DeclareWvVector2(_classname_, _type_)  \
    typedef class WvVector<_type_> _classname_

#define DeclareWvVector(_type_) DeclareWvVector2(_type_##Vector, _type_)

#endif // __WVVECTOR_H
