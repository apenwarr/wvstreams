/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * An iterator that can sort anything that has an iterator, includes the
 * right member functions, and uses WvLink objects - at the moment,
 * this includes WvList- and WvHashTable-based objects.
 */
#ifndef __WVSORTER_H
#define __WVSORTER_H

#include "wvlink.h"

extern WvLink blank_wvlink;

// the base class for sorted list iterators.
// It is similar to IterBase, except for rewind(), next(), and cur().
// The sorting is done in rewind(), which makes an array of WvLink
// pointers and calls qsort.  "lptr" is a pointer to the current WvLink *
// in the array, and next() increments to the next one.
// NOTE: we do not keep "prev" because it makes no sense to do so.
//       I guess Sorter::unlink() will be slow... <sigh>
class WvSorterBase
{
public:
    typedef int (CompareFunc)(const void *a, const void *b);
    
    void *list;
    WvLink **array;
    WvLink **lptr;
    
    WvSorterBase(void *_list)
    	{ list = _list; array = lptr = NULL; }
    ~WvSorterBase()
    	{ if (array) delete array; }
    WvLink *next()
	{ return lptr ? *(++lptr) : *(lptr = array); }
    WvLink *cur()
    	{ return lptr ? *lptr : &blank_wvlink; }
    
protected:
    template <class _list_,class _iter_> void rewind(CompareFunc *cmp);
    
    static int magic_compare(const void *_a, const void *_b);
    static CompareFunc *actual_compare;
};

// the actual type-specific sorter.  Set _list_ and _iter_ to be your
// common base class (eg. WvListBase and WvListBase::IterBase) if possible,
// so we don't need to generate a specific rewind(cmp) function for each
// specific type of list.  Since rewind(cmp) is the only non-inline function
// in a sorter, that means you only need one of them per top-level container
// type (ie. one for WvList and one for HashTable), not one per data type
// you might store in such a container.
template <class _type_,class _list_,class _iter_>
class WvSorter : public WvSorterBase
{
public:
    typedef int (RealCompareFunc)(const _type_ *a, const _type_ *b);
    RealCompareFunc *cmp;
    
    WvSorter(_list_ &_list, RealCompareFunc *_cmp)
	: WvSorterBase(&_list)
	{ cmp = _cmp; }
    _type_ *ptr() const
	{ return (_type_ *)(*lptr)->data; }
    
    // declare standard iterator accessors
    WvIterStuff(_type_);
    
    //void rewind()
    //	{ WvSorterBase::rewind((CompareFunc *)cmp); }
    
    void rewind()
      { WvSorterBase::rewind<_list_,_iter_>((CompareFunc *)cmp); }
};


// Note that this is largely the same as WvLink::SorterBase::rewind(),
// except we iterate through a bunch of lists instead of a single one.
template <class _list_,class _iter_>
void WvSorterBase::rewind(CompareFunc *cmp)
{
    if (array)
        delete array;
    array = lptr = NULL;

    int n = ((_list_ *)list)->count();
    array = new WvLink * [n+1];
    WvLink **aptr = array;

    // fill the array with data pointers for sorting, so that the user doesn't
    // have to deal with the WvLink objects.  Put the WvLink pointers back 
    // in after sorting.
    _iter_ i(*(_list_ *)list);
    aptr = array;
    for (i.rewind(); i.next(); )
    {
        *aptr = i.cur();
        aptr++;
    }
    
    *aptr = NULL;

    // sort the array.  "Very nearly re-entrant" (unless the compare function
    // ends up being called recursively or something really weird...)
    CompareFunc *old_compare = actual_compare;
    actual_compare = cmp;
    qsort(array, n, sizeof(WvLink *), magic_compare);
    actual_compare = old_compare;

    lptr = NULL;    // subsequent next() will set it to first element.
}


#endif // __WVSORTER_H
