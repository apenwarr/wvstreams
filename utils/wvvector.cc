/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides a dynamic array data structure.
 */
#include "wvvector.h"

/***** WvVectorBase *****/

WvVectorBase::WvVectorBase() :
    xseq(NULL), xsize(0), xslots(0)
{
}


int WvVectorBase::growcapacity(int minslots)
{
    int newslots = xslots != 0 || minslots == 0 ?
        xslots : MINALLOC;
    while (newslots < minslots)
        newslots *= 2;
    return newslots;
}


int WvVectorBase::shrinkcapacity(int maxslots)
{
    maxslots *= 2;
    int newslots = xslots;
    while (newslots > maxslots)
        newslots /= 2;
    return newslots;
}
