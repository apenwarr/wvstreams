/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides an auto-resizing array data structure.
 */
#ifndef __WVVECTOR_H
#define __WVVECTOR_H

#include "wvlink.h"
#include <string.h>

/**
 * The untyped vector base type.
 * @see WvVector
 */
class WvVectorBase
{
protected:
    static const int MINALLOC = 4;
        /*!< the minimum number of slots to allocate */

    void **xseq; /*!< the controlled sequence */
    int xcount; /*!< the number of elements in the sequence */
    int xslots; /*!< the capacity of the array */
    bool auto_free; /*!< whether to auto-delete the elements when removed */

    /** Creates an empty vector. */
    WvVectorBase(bool _auto_free);

    /** Computes the number of slots needed to grow to at least minslots. */
    int growcapacity(int minslots);
    
    /** Computes the number of slots needed to shrink down to maxslots. */
    int shrinkcapacity(int maxslots);
    
    /** A shorthand for memmove() with size adjustment. */
    void moveelems(void *dst, void *src, int nelems)
        { memmove(dst, src, nelems * sizeof(void *)); }

    /** Removes the element at the specified slot. */
    void remove(int slot);
    
    /** Inserts an element at the specified slot. */
    void insert(int slot, void *elem);

    /** Appends an element onto the tail of the vector. */
    void append(void *elem);
    
public:
    /** Returns the number of elements actually stored in the vector. */
    int count() const
        { return xcount; }

    /** Returns true if the vector is empty. */
    bool isempty() const
        { return xcount == 0; }

    /** The number of elements that could be stored without resizing. */
    int capacity() const
        { return xslots; }
    
    /**
     * Adjusts the capacity of the vector.
     *
     * If the new capacity is greater than the old one, extends the array
     * size without actually filling in any elements.
     */
    void setcapacity(int newslots);

    /** Compacts the vector to minimize its footprint. */
    void compact()
        { setcapacity(count()); }
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
    WvVector(bool _auto_free) : WvVectorBase(_auto_free)
        { }

    /** Destroys the vector and all of its contents. */
    ~WvVector()
        { zap(); }

    /** Dereferences a particular slot of the vector. */
    T *operator[] (int slot)
        { return ptr()[slot]; }

    /** Removes all elements from the vector. */
    void zap()
    { 
        // guard against potential side-effects
        T **oldarray = ptr();
        int oldcount = xcount;
        xcount = 0;
        xslots = 0;
        xseq = NULL;
	if (auto_free)
	{
            while (oldcount > 0)
		delete oldarray[--oldcount];
	}
        delete[] oldarray;
    }

    void remove(int slot, bool never_delete = false)
    {
	T *obj = (*this)[slot];
	WvVectorBase::remove(slot);
	if (auto_free && !never_delete)
	    delete obj;
    }
    
    /** Removes the last element */
    void remove_last()
        { if (xcount) { remove(xcount-1); } }
	
    T *last()
        { return xcount ? (*this)[xcount-1] : NULL; }
    
    void insert(int slot, T *elem)
        { WvVectorBase::insert(slot, elem); }

    void append(T *elem)
        { WvVectorBase::append(elem); }
    
    // FIXME: I'd rather not give public access to this, since it's dangerous!
    T **ptr() 
        { return reinterpret_cast<T **>(xseq); }
    
    
    /** A simple iterator that walks through all elements in the list. */
    class Iter
    {
	WvVector<T> *list;
	int count;
	
    protected:
	/** _list is allowed to be NULL, and this will still work. */
	Iter(WvVector<T> *_list) : list(_list)
	    { count = -1; }

    public:
	Iter(WvVector<T> &_list) : list(&_list)
	    { count = -1; }
	
	void rewind()
	    { count = -1; }
	bool next()
	    { count++; return cur(); }
	bool cur()
	    { return list && count >= 0 && count < list->count(); }
	
	T *ptr() const
	    { return (*list)[count]; }
	
	WvIterStuff(T);
    };
};

#endif // __WVVECTOR_H
