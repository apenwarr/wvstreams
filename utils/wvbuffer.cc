/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
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
	i.data().zap();
    inuse = 0;
}


unsigned char *WvBuffer::get(size_t num)
{
    WvMiniBufferList::Iter i(list);
    WvMiniBuffer *firstb, *b, *destb;
    size_t got, avail;

    if (inuse < num  ||  num == 0)
	return NULL;
    
    assert(inuse >= num); // so there must be enough buffers
    
    inuse -= num;
    
    i.rewind(); i.next();
    
    // if the first minibuffer is empty, delete it.
    firstb = &i.data();
    if (firstb->used() == 0)
    {
	Dprintf("<del-0 MiniBuffer(%d)\n", firstb->total());
	i.unlink();
    }

    // if the (new) first minibuffer has enough data, just use that.
    firstb = &i.data();
    if (firstb->used() >= num)
	return firstb->get(num);

    // nope.  Is there enough wasted room in this buffer for the rest of
    // the data?
    if (firstb->free() >= num - firstb->used())
    {
	got = firstb->used();
	destb = firstb;
    }
    else    // must allocate a new "first" buffer to hold entire 'num' bytes
    {
	got = 0;
	list.prepend(destb = new WvMiniBuffer(num), true);
	Dprintf("<new-1 MiniBuffer(%d)>\n", num);
    }

    for (i.rewind(), i.next(); i.cur(); )
    {
	b = &i.data();
	if (b == destb)
	{
	    i.next();
	    continue;
	}
	
	if (b->used() > num - got)
	    avail = num - got;
	else
	    avail = b->used();
	got += avail;
	
	destb->put(b->get(avail), avail);
	if (!b->used())
	{
	    Dprintf("<del-1 MiniBuffer(%d)\n", b->total());
	    i.unlink();
	}
	else
	    i.next();
    }
    
    return destb->get(num);
}


void WvBuffer::unget(size_t num)
{
    WvMiniBuffer *b;
    WvMiniBufferList::Iter i(list);
    size_t ungettable;
    
    i.rewind(); i.next();
    b = &i();
    
    ungettable = b->total() - b->used() - b->free();
    
    if (num > ungettable)
	num = ungettable;
    
    b->unget(num);
    inuse += num;
}


unsigned char *WvBuffer::alloc(size_t num)
{
    WvMiniBuffer *lastb, *b;
    size_t newsize;
    
    if (list.tail && list.tail->data)
    {
	lastb = (WvMiniBuffer *)list.tail->data;
	if (lastb->free() >= num)
	{
	    inuse += num;
	    return lastb->alloc(num);
	}
    }
    else
	lastb = NULL;
    
    // otherwise, we need a new MiniBuffer so we can provide contiguous 'num'
    // bytes.  New buffers grow in size exponentially, and have minimum size
    // of 10.
    newsize = 0;
    if (lastb)
    {
	newsize = lastb->total();
	if (lastb->used() > lastb->total() / 2)
	    newsize *= 2;
    }
    if (newsize < 10)
	newsize = 10;
    if (newsize < num)
	newsize = num;
    b = new WvMiniBuffer(newsize);
    Dprintf("<new-2 MiniBuffer(%d)>\n", newsize);
    
    list.append(b, true);
    
    inuse += num;
    return b->alloc(num);
}


void WvBuffer::unalloc(size_t num)
{
    WvMiniBuffer *lastb, *b;
    
    assert(inuse >= num);
    
    if (inuse < num  ||  num == 0)
	return; // strange
    
    inuse -= num;
    
    // fast track:  if enough bytes are in the very last minibuffer (the
    // usual case) then we can unalloc() it quickly.
    lastb = (WvMiniBuffer *)list.tail->data;
    if (lastb->used() >= num)
    {
	lastb->unalloc(num);
	return;
    }
	
    // free up as much as we can from the last buffer, counting backwards
    // each time.  This is slow because we have only a singly-linked list.
    WvMiniBufferList::Iter i(list);
    
    while (num > 0)
    {
	// iterate through to the last element
	for (i.rewind(); i.next() && i.cur()->next; )
	    ;
	
	b = &i.data();
	
	if (b->used() < num)
	{
	    num -= b->used();
	    Dprintf("<del-2 MiniBuffer(%d)>\n", b->total());
	    i.unlink();
	}
	else
	{
	    // only unalloc part of the now-last element
	    b->unalloc(num);
	    num = 0;
	}
    }
}


/*
 * we could do this with alloc() and memcpy(), but that can waste space. If
 * we know the data already, append whatever fits to the last buffer, then
 * add any remaining data to a new one.
 */
void WvBuffer::put(const void *data, size_t num)
{
    WvMiniBuffer *lastb, *b;
    size_t newsize;
    
    inuse += num;

    // use up any space in the currently-last minibuffer
    if (list.tail && list.tail->data)
    {
	lastb = (WvMiniBuffer *)list.tail->data;
	
	newsize = lastb->free() >= num ? num : lastb->free();
	lastb->put(data, newsize);
	num -= newsize;
	data = (char *)data + newsize;
    }
    else
	lastb = NULL;
    
    // otherwise, we need a new MiniBuffer so we can provide contiguous 'num'
    // bytes.  New buffers grow in size exponentially, and have minimum size
    // of 10.
    if (num > 0)
    {
	newsize = 0;
	if (lastb)
	{
	    newsize = lastb->total();
	    if (lastb->used() >= lastb->total() / 2)
		newsize *= 2;
	}
	if (newsize < 10)
	    newsize = 10;
	if (newsize < num)
	    newsize = num;
	b = new WvMiniBuffer(newsize);
	Dprintf("<new-3 MiniBuffer(%d)>\n", newsize);
	
	list.append(b, true);
	b->put(data, num);
    }
}


void WvBuffer::put(const WvString &str)
{
    if (!!str)
	put(str, strlen(str));
}


WvString WvBuffer::getstr()
{
    WvString s;
    size_t len = used();
    s.setsize(len + 1);
    
    char *cptr = s.edit();
    memcpy(cptr, get(len), len);
    cptr[len] = 0;
    
    return s;
}


size_t WvBuffer::strchr(unsigned char ch)
{
    WvMiniBufferList::Iter i(list);
    size_t off = 0, t;
    
    for (i.rewind(); i.next(); )
    {
	WvMiniBuffer &b = i;
	
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
	WvMiniBuffer &b = i;
	
	t = b.match(chlist, numch, reverse);
	
	if (t < b.used())
	    return off + t; // done
	else
	    off += b.used();
    }
    
    return off;
}
