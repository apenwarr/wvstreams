/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * An iterator that can sort anything that has an Iter subclass with the
 * right member functions.
 * 
 * See wvsorter.h.
 */
#include "wvsorter.h"

WvSorterBase::CompareFunc *WvSorterBase::actual_compare;

int WvSorterBase::magic_compare(const void *_a, const void *_b)
{
    void *a = *(void **)_a, *b = *(void **)_b;
    return actual_compare(a, b);
}


