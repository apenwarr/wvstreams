/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides a dynamic array data structure.
 */
#include "wvvector.h"
#include <assert.h>

WvVectorBase::comparison_type_t WvVectorBase::innercomparator;

WvVectorBase::WvVectorBase(int slots)
    : xseq(NULL), xcount(0), xslots(0)
{
    innercomparator = NULL;
    set_capacity(slots);
}

void WvVectorBase::remove(int slot)
{
    --xcount;
    if (xcount - slot)
	memmove(xseq + slot, xseq + slot + 1,
		(xcount - slot) * sizeof(WvLink *));
}

void WvVectorBase::insert(int slot, WvLink *elem)
{
    if (++xcount > xslots)
    {
	xslots *= 2;
	set_capacity(xslots);
    }
    memmove(xseq + slot + 1, xseq + slot,
	    (xcount - slot - 1) * sizeof(WvLink *));
    xseq[slot] = elem;
}

void WvVectorBase::append(WvLink *elem)
{
    if (++xcount > xslots)
    {
	xslots *= 2;
	set_capacity(xslots);
    }
    xseq[xcount - 1] = elem;
}

void WvVectorBase::set_capacity(int newslots)
{
    // Ensure we don't eliminate data when we shrink
    if (newslots < xcount)
	newslots = xcount;

    // Free the memory if we don't want any.
    if (newslots <= 0)
    {
	xslots = 0;
	free(xseq);
	xseq = NULL;
	return;
    }

    // Allocate memory, if we want it
    xslots = newslots;
    void *newseq = realloc(xseq, xslots * sizeof(WvLink *));
    assert(newseq != NULL || xslots == 0);
    if (newseq != NULL || xslots == 0)
	xseq = static_cast<WvLink **>(newseq);
}

WvLink *WvVectorBase::IterBase::find(const void *data)
{
    for (rewind(); next(); )
    {
	if (link->data == data)
	    break;
    }
    return link;
}

WvLink *WvVectorBase::IterBase::find_next(const void *data)
{
    if (link)
    {
	if (link->data == data)
	    return link;

	for (; next(); )
	{
	    if (link->data == data)
		break;
	}
    }
    return link;
}
