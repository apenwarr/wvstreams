/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides a dynamic array data structure.
 */
#ifndef __WVVECTOR_H
#define __WVVECTOR_H

#include <stdlib.h>
#include <string.h>
#include <new>

/**
 * Block operations help us to reduce the amount of overhead
 * for maintaining controlled sequences of non-object types.
 *
 * The base type is provided mostly for documentation purposes.
 */
template<class T>
struct BlockOps
{
    /**
     * Fills an array with initialized elements.
     * @param target the pointer to the uninitialized array
     * @param n the number of elements
     */
    static void initelems(T *target, size_t n)
    {
    }

    /**
     * Destroys the initialized elements in an array.
     * @param target the pointer to the initialized array
     * @param n the number of elements
     */
    static void destroyelems(T *target, size_t n)
    {
    }

    /**
     * Moves an initialized array of elements into a target
     * uninitialized array leaving the source array uninitialized.
     * The arrays may overlap but must not coincide.
     *
     * @param target the pointer to the uninitialized target array
     * @param source the pointer to the initialized source array
     * @param n the number of elements
     */
    static void moveelems(T *target, T *source, size_t n)
    {
        memmove(target, source, n * sizeof(T));
    }

    /**
     * Moves an initialized array of elements into a target
     * uninitialized array without affecting the source array.
     * @param target the pointer to the uninitialized target array
     * @param source the pointer to the initialized source array
     * @param n the number of elements
     */
    static void copyelems(T *target, const T *source, size_t n)
    {
        memcpy(target, source, n * sizeof(T));
    }
};


/**
 * Shallow block operations for use with primitive types.
 */
template<class T>
struct ShallowBlockOps : public BlockOps<T>
{
};


/**
 * Deep block operations for use with object types that have
 * a visible default constructor, copy constructor and destructor.
 */
template<class T>
struct DeepBlockOps : public BlockOps<T>
{
    static void initelems(T *target, size_t n)
    {
        while (n-- > 0)
            new(target + n) T();
    }

    static void destroyelems(T *target, size_t n)
    {
        while (n-- > 0)
            (target + n)->~T();
    }

    static void moveelems(T *target, T *source, size_t n)
    {
        if (source < target)
        {
            while (n-- > 0)
            {
                const T &ref = *(source + n);
                new(target + n) T(ref);
                (source + n)->~T();
            }
        }
        else
        {
            for (size_t i = 0; i < n; ++i)
            {
                const T &ref = *(source + i);
                new(target + i) T(ref);
                (source + i)->~T();
            }
        }
    }

    static void copyelems(T *target, const T *source, size_t n)
    {
        while (n-- > 0)
        {
            const T &ref = *(source + n);
            new(target + n) T(ref);
        }
    }
};


/**
 * The untyped vector base type.
 * @see WvVector
 */
class WvVectorBase
{
protected:
    static const int MINALLOC = 4;
        /*!< the minimum number of slots to allocate */

    void *xseq; /*!< the controlled sequence */
    int xsize; /*!< the number of elements in the sequence */
    int xslots; /*!< the capacity of the array */

    /**
     * Creates an empty vector.
     */
    WvVectorBase();

    /**
     * Computes the number of slots needed to grow on demand.
     * @param newslots the minimum number of slots currently needed
     * @return the recommended number of slots
     */
    int growcapacity(int minslots);
    
    /**
     * Computes the number of slots needed to shrink on demand.
     * @param newslots the maximum number of slots currently needed
     * @return the recommended number of slots
     */
    int shrinkcapacity(int maxslots);

public:
    /**
     * Returns the size of the vector.
     * @return the number of elements actually stored
     */
    inline int size() const
    {
        return xsize;
    }

    /**
     * Returns true if the vector is empty.
     * @return true if the vector is empty
     */
    inline bool isempty() const
    {
        return xsize == 0;
    }

    /**
     * Returns the current capacity of the vector.
     * @return the number of elements that could be stored without
     *         resizing
     */
    inline int capacity() const
    {
        return xslots;
    }
};


/**
 * A dynamic array data structure with constant time lookup,
 * linear time insertion / removal, and expected logarithmic time
 * append.
 *
 * @param T the element type
 * @param BOps a structure defining block operations on elements
 */
template<class T, class BOps>
class WvVector : public WvVectorBase
{
    typedef T& TRef;
    typedef const T& ConstTRef;

    void _setsize(int newsize);
    void _setcapacity(int newsize);
    
public:
    /**
     * Creates an empty vector.
     */
    WvVector()
    {
    }
    
    /**
     * Creates a vector of the specified size with default contents.
     * @param size the number of slots
     */
    WvVector(int size)
    {
        setsize(size);
    }

    /**
     * Destroys the vector and all of its contents.
     */
    ~WvVector()
    {
        setcapacity(0);
    }

    /**
     * Returns a pointer to the beginning of the controlled sequence.
     * @return the controlled sequence, may be NULL if size() == 0
     */
    inline const T *ptr() const
    {
        return static_cast<const T*>(xseq);
    }

    /**
     * Returns a pointer to the beginning of the controlled sequence.
     * @return the controlled sequence, may be NULL if size() == 0
     */
    inline T *ptr()
    {
        return static_cast<T*>(xseq);
    }
    
    /**
     * Dereferences a particular slot of the vector.
     * @param slot the slot
     * @return the element reference
     */
    inline TRef operator[] (int slot)
    {
        return ptr()[slot];
    }
    
    /**
     * Dereferences a particular slot of the vector.
     * @param slot the slot
     * @return the element reference
     */
    inline ConstTRef operator[] (int slot) const
    {
        return ptr()[slot];
    }


    /**
     * Adjusts the size of the vector.
     * <p>
     * If the new size is greater than the old one, appends
     * newly initialized elements until the desired size has been
     * reached.  Otherwise destroys and removes old elements from
     * the tail.
     * </p>
     * @param newsize the desired size of the vector
     */
    inline void setsize(int newsize)
    {
        if (newsize == xsize)
            return;
        _setsize(newsize);
    }

    /**
     * Adjusts the capacity of the vector.
     * <p>
     * If the new capacity is greater than the old one, extends
     * the controlled sequence but does not append any elements.
     * Otherwise shrinks the controlled sequence and destroys
     * initialized elements from the tail until the desired size
     * has been reached.
     * </p>
     * @param slots the desired capacity of the vector
     */
    inline void setcapacity(int newslots)
    {
        if (newslots == xslots)
            return;
        _setcapacity(newslots);
    }

    /**
     * Compacts the vector to minimize its footprint.
     */
    inline void compact()
    {
        setcapacity(size());
    }

    /**
     * Removes all elements from the vector.
     */
    inline void zap()
    {
        setsize(0);
    }

    /**
     * Removes the element at the specified slot.
     * @param slot the slot
     */
    void remove(int slot)
    {
        xsize -= 1;
        BOps::destroyelems(ptr() + slot, 1);
        BOps::moveelems(ptr() + slot, ptr() + slot + 1,
            xsize - slot);
        setcapacity(shrinkcapacity(xsize));
    }

    /**
     * Inserts an element at the specified slot.
     * @param slot the slot
     * @param elem the element
     */
    void insert(int slot, ConstTRef elem)
    {
        // FIXME: suboptimal, might perform two copies
        setcapacity(growcapacity(xsize + 1));
        BOps::moveelems(ptr() + slot + 1, ptr() + slot,
            xsize - slot);
        BOps::copyelems(ptr() + slot, & elem, 1);
        xsize += 1;
    }

    /**
     * Appends an element onto the tail of the vector.
     * @param elem the element
     */
    void pushback(ConstTRef elem)
    {
        setcapacity(growcapacity(xsize + 1));
        BOps::copyelems(ptr() + xsize, & elem, 1);
        xsize += 1;
    }

    /**
     * Prepends an element onto the head of the vector.
     * @param elem the element
     */
    inline void pushfront(ConstTRef elem)
    {
        insert(0, elem);
    }

    /**
     * Removes and returns the first element of the vector.
     * @return the element
     */
    T popfront()
    {
        T value = ptr()[0];
        remove(0);
        return value;
    }

    /**
     * Removes and returns the last element of the vector.
     * @return the element
     */
    T popback()
    {
        T value = ptr()[xsize - 1];
        setsize(xsize - 1);
        return value;
    }
};

template<class T, class BOps>
void WvVector<T, BOps>::_setsize(int newsize)
{
    if (newsize > xsize)
    {
        setcapacity(growcapacity(newsize));
        BOps::initelems(ptr() + xsize, newsize - xsize);
        xsize = newsize;
    }
    else
    {
        BOps::destroyelems(ptr() + newsize, xsize - newsize);
        xsize = newsize;
        setcapacity(shrinkcapacity(newsize));
    }
}


template<class T, class BOps>
void WvVector<T, BOps>::_setcapacity(int newslots)
{
    if (newslots < xsize)
    {
        BOps::destroyelems(ptr() + newslots, xsize - newslots);
        xsize = newslots;
    }
    T *oldseq = ptr();
    xslots = newslots;
    if (newslots != 0)
    {
        xseq = malloc(sizeof(T) * newslots);
        BOps::moveelems(ptr(), oldseq, xsize);
    }
    else
        xseq = NULL;
    free(oldseq);
}


#endif // __WVVECTOR_H
