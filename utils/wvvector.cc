/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides a dynamic array data structure.
 */
#include "wvvector.h"
#include <assert.h>


WvVectorBase::WvVectorBase(bool _auto_free)
{
    xseq = NULL;
    xcount = xslots = 0;
    autofree = _auto_free;
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


void WvVectorBase::remove(int slot)
{
    xcount--;
    moveelems(xseq + slot, xseq + slot + 1, xcount - slot);
    setcapacity(shrinkcapacity(xcount));
}


void WvVectorBase::insert(int slot, void *elem)
{
    setcapacity(growcapacity(xcount + 1));
    moveelems(xseq + slot + 1, xseq + slot, xcount - slot);
    xseq[slot] = elem;
    xcount++;
}


void WvVectorBase::append(void *elem)
{
    setcapacity(growcapacity(xcount + 1));
    xseq[xcount] = elem;
    xcount++;
}


void WvVectorBase::setcapacity(int newslots)
{
    if (newslots == xslots)
	return;
    
    assert(newslots >= xcount);
    if (newslots < xcount)
	xcount = newslots;
    void **oldseq = xseq;
    xslots = newslots;
    if (newslots != 0)
    {
	xseq = new void *[newslots];
	moveelems(xseq, oldseq, xcount);
    }
    else
	xseq = NULL;
    deletev oldseq;
}
