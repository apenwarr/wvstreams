/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Declarations for WvMiniBuffer (a statically-sized data buffer with
 * get/put functions) and WvBuffer (a dynamically-sized buffer made from
 * a list of WvMiniBuffers).
 *
 * All these functions are described in wvbuffer.h
 */
#include "wvbuffer.h"
#include <assert.h>
#include <stdio.h> // for printf()

#if 0
#define Dprintf printf
#else
#define Dprintf if (0) printf
#endif

#define MINSIZE 256

///////////////////// WvMiniBuffer


size_t WvMiniBuffer::strchr(unsigned char ch) const
{
    unsigned char *cptr;
    
    for (cptr = head; cptr < tail; cptr++)
	if (*cptr == ch) return cptr - head + 1;
    
    return 0;
}


size_t WvMiniBuffer::match(const unsigned char chlist[], size_t numch,
			     bool reverse) const
{
    const unsigned char *cptr, *cp2, *cpe = chlist + numch;
    
    for (cptr = head; cptr < tail; cptr++)
    {
	if (reverse)
	{
	    // stop if this character matches ANY of chlist
	    for (cp2 = chlist; cp2 < cpe; cp2++)
		if (*cptr == *cp2) return cptr - head;
	}
	else
	{
	    // stop if this character matches NONE of chlist
	    for (cp2 = chlist; cp2 < cpe; cp2++)
		if (*cptr == *cp2) break;
	    if (cp2 == cpe) return cptr - head;
	}
    }

    return cptr - head;
}


////////////////////// WvBuffer


void WvBuffer::zap()
{
    WvMiniBufferList::Iter i(list);
    
    for (i.rewind(); i.next(); )
	i->zap();
    inuse = 0;
}


unsigned char *WvBuffer::get(size_t num)
{
    if (num == 0 || num > inuse)
        return NULL; // insufficient data
    inuse -= num;

    // search for the first non-empty minibuffer
    WvMiniBuffer *firstb;
    for (;;)
    {
        assert(list.head.next && list.head.next->data); // true if inuse is not 0
        firstb = (WvMiniBuffer *)list.head.next->data;
        if (firstb->used() != 0) break;
        Dprintf("<del-0 MiniBuffer(%d)\n", firstb->total());
        list.unlink_first();
    }

    // if we have enough data, return it now
    if (firstb->used() >= num)
        return firstb->get(num);
    
    // is there enough space in this buffer to hold the rest of the data?
    size_t needed = num - firstb->used();
    if (firstb->free() < needed)
    {
        // must allocate a new "first" buffer to hold entire 'num' bytes
        firstb = new WvMiniBuffer(num);
        list.prepend(firstb, true);
        Dprintf("<new-1 MiniBuffer(%d)>\n", num);
        needed = num;
    }

    // grab data from subsequent minibuffers, delete emptied ones
    WvMiniBufferList::Iter i(list);
    i.rewind(); i.next();
    for (;;)
    {
        i.next();
        assert(i.cur() && i.ptr());
        WvMiniBuffer *b = i.ptr();
        
        // move data from current buffer
        size_t chunk = b->used() < needed ? b->used() : needed;
        firstb->put(b->get(chunk), chunk);
        needed -= chunk;
        if (needed == 0)
            break;

        // buffer is now empty so delete it
        Dprintf("<del-1 MiniBuffer(%d)>\n", b->total());
        i.xunlink();
    }
    return firstb->get(num);
}


void WvBuffer::unget(size_t num)
{
    if (num == 0)
        return;
    inuse += num;

    // we're asserting here if the unget cannot be performed
    // to help catch serious bugs that might otherwise result if we just
    // ungot some smaller amount
    assert(list.head.next && list.head.next->data); // must have a head
    WvMiniBuffer *b = (WvMiniBuffer *)list.head.next->data;

    size_t ungettable = b->total() - b->used() - b->free();
    assert(num <= ungettable);
    b->unget(num);
}


unsigned char *WvBuffer::alloc(size_t num)
{
    if (num == 0)
        return NULL;
    inuse += num;

    // check last buffer for enough contiguous space
    if (list.tail && list.tail->data)
    {
        WvMiniBuffer *lastb = (WvMiniBuffer *)list.tail->data;
        if (lastb->free() >= num)
            return lastb->alloc(num);

        // if the last buffer was empty, delete it since it will be wasted
        if (lastb->used() == 0)
        {
            WvMiniBufferList::Iter i(list);
            for (i.rewind(); i.next() && i.cur()->next; ) ;
            Dprintf("<del-2 MiniBuffer(%d)>\n", lastb->total());
            i.xunlink();
        }
    }
    // otherwise create a new buffer
    WvMiniBuffer *newb = append_new_buffer(num);
    return newb->alloc(num);
}


void WvBuffer::put(const void *data, size_t num)
{
    if (num == 0)
        return;
    inuse += num;

    // use up any space in the currently-last minibuffer
    if (list.tail && list.tail->data)
    {
        WvMiniBuffer *lastb = (WvMiniBuffer *)list.tail->data;
        size_t chunk = lastb->free() < num ? lastb->free() : num;
        lastb->put(data, chunk);
        num -= chunk;
        if (num == 0)
            return;
        data = (char *)data + chunk;
    }
    // create a new buffer for the remainder
    WvMiniBuffer *newb = append_new_buffer(num);
    newb->put(data, num);
}


WvMiniBuffer *WvBuffer::append_new_buffer(size_t minsize)
{
    // new buffers grow in size exponentially,
    size_t newsize = 0;
    if (list.tail && list.tail->data)
    {
        WvMiniBuffer *lastb = (WvMiniBuffer *)list.tail->data;
        newsize = lastb->total();
        if (lastb->used() > lastb->total() / 2)
            newsize *= 2;
    }
    // minimum size of MINSIZE or minsize, whichever is greater
    if (newsize < MINSIZE)
        newsize = MINSIZE;
    if (newsize < minsize)
        newsize = minsize;

    WvMiniBuffer *newb = new WvMiniBuffer(newsize);
    Dprintf("<new-2 MiniBuffer(%d)>\n", newsize);
    list.append(newb, true);
    return newb;
}


void WvBuffer::unalloc(size_t num)
{
    assert(inuse >= num);
    if (num == 0)
        return;
    inuse -= num;
    
    assert(list.tail && list.tail->data); // true if inuse is not 0
    WvMiniBuffer *lastb = (WvMiniBuffer *)list.tail->data;
    for (;;)
    {
        // unalloc from end of last buffer
        size_t chunk = lastb->used() < num ? lastb->used() : num;
        lastb->unalloc(chunk);
        num -= chunk;
        if (num == 0)
            break;

        // last buffer is now empty so delete it and find previous
        WvMiniBufferList::Iter i(list);
        for (i.rewind(); i.next() && i.cur()->next; ) ;
        Dprintf("<del-3 MiniBuffer(%d)>\n", lastb->total());
        i.xunlink();
        assert(i.cur() && i.ptr()); // if NULL, then counts were wrong
        lastb = i.ptr();
    }
}


void WvBuffer::put(WvStringParm str)
{
    if (!!str)
	put(str, strlen(str));
}


// to merge another WvBuffer into this one, simply move all of its
// WvMiniBuffer objects into our own list.
void WvBuffer::merge(WvBuffer &buf)
{
    // move the buffers
    WvMiniBufferList::Iter i(buf.list);
    for (i.rewind(); i.next(); )
    {
        i.cur()->auto_free = false;
        list.append(&i(), true);
    }
    buf.list.zap();

    // adjust the used space counters
    inuse += buf.used();
    buf.inuse = 0;
}


WvFastString WvBuffer::getstr()
{
    // add a terminating NUL, in case there isn't one
    put("", 1);
    
    // grab the string and return it.  If nobody ever assigns it to a WvString
    // (as opposed to a WvStringParm), there's no need to ever copy the string!
    return (const char *)get(used());
}


size_t WvBuffer::strchr(unsigned char ch)
{
    WvMiniBufferList::Iter i(list);
    size_t off = 0, t;
    
    for (i.rewind(); i.next(); )
    {
	WvMiniBuffer &b = *i;
	
	t = b.strchr(ch);
	
	if (t)
	    return off + t; // found it
	else
	    off += b.used();
    }
    
    return 0;
}


size_t WvBuffer::match(const unsigned char chlist[], size_t numch,
			 bool reverse)
{
    WvMiniBufferList::Iter i(list);
    size_t off = 0, t;
    
    for (i.rewind(); i.next(); )
    {
	WvMiniBuffer &b = *i;
	
	t = b.match(chlist, numch, reverse);
	
	if (t < b.used())
	    return off + t; // done
	else
	    off += b.used();
    }
    
    return off;
}
