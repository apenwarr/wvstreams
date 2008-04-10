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

WvLink blank_wvlink(NULL, false, "blank_wvlink");
WvSorterBase::CompareFunc *WvSorterBase::actual_compare;

int WvSorterBase::magic_compare(const void *_a, const void *_b)
{
    WvLink *a = *(WvLink **)_a, *b = *(WvLink **)_b;
    return actual_compare(a->data, b->data);
}


