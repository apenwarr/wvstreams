/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * Implementation of a Linked List management class, or rather, macros that
 * declare arbitrary linked list management classes.
 * 
 * wvlinklist.h does all the real work.
 */
#include "wvlinklist.h"

WvLink::WvLink(void *_data, WvLink *prev, WvLink *&tail, bool _auto_free,
	       char *_id)
{
    data = _data;
    next = prev->next;
    if (!next) tail = this;
    prev->next = this;
    auto_free = (unsigned)_auto_free;
    id = _id;
}


size_t WvList::count() const
{
    WvLink *l;
    size_t n = 0;
    
    for (l = head.next; l; l = l->next)
	n++;
    return n;
}


WvLink *WvList::IterBase::find(const void *data)
{
    for (rewind(); next(); )
    {
	if (link->data == data)
	    break;
    }
    
    return link;
}


void WvList::SorterBase::rewind( int (*cmp)( const void *, const void * ) )
{
    if( array )
        delete array;
    array = lptr = NULL;

    int n = list->count();
    void ** varray = new void * [n+1];
    array = (WvLink **) varray;

    // fill the array with data pointers for sorting, so that the user doesn't
    // have to deal with the WvLink objects.  Put the WvLink pointers back 
    // in after sorting.
    WvLink * src = list->head.next;
    void ** vptr = varray;
    while( src ) {
        *vptr = src->data;
        vptr++;
        src = src->next;
    }
    *vptr = NULL;

    // sort the array
    qsort( array, n, sizeof( WvLink * ), cmp );

    // go through the array, find the WvLink corresponding to each element,
    // and substitute.
    src = list->head.next;
    while( src ) {
        lptr = (WvLink **) vptr = varray;
        while( lptr != NULL ) {
            if( *vptr == src->data ) {
                *lptr = src;
                break;
            }
            lptr++;
            vptr++;
        }
        src = src->next;
    }

    lptr = NULL;    // subsequent next() will set it to first element.
}
