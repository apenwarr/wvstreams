/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Declarations for WvMiniBuffer (a statically-sized data buffer with
 * get/put functions) and WvBuffer (a dynamically-sized buffer made from
 * a list of WvMiniBuffers).
 */
#ifndef __WVBUFFER_H
#define __WVBUFFER_H

#include <string.h>
#include "wvlinklist.h"
#include "wvstring.h"
#include <string.h>


class WvMiniBuffer
{
    unsigned char *buffer, *head, *tail;
    size_t size;
    
public:
    WvMiniBuffer(size_t _size)
	{ buffer = head = tail = new unsigned char[(size = _size) + 16]; }
    ~WvMiniBuffer()
        { delete[] buffer; }
    
    /*
     * return number of bytes total/used/left in minibuffer. Note that
     * used+left != total, because bytes cannot be "recycled" until we do a
     * zap().  That is, the sequence put(15); get(15); causes used() to
     * return the same value as before, and free() to be reduced by 15 bytes.
     */
    size_t total() const
        { return size; }
    size_t used() const
        { return tail - head; }
    size_t free() const
        { return buffer + size - tail; }
    
    /*
     * remove all data from the minibuffer by setting head/tail to buffer
     * start
     */
    void zap()
       { head = tail = buffer; }
    
    
    /*
     * NO ERROR CHECKING in any of the following!!
     */
   

    /*
     * return a pointer to the next 'num' bytes in the minibuffer; valid
     * until buffer is zapped.
     */
    unsigned char *get(size_t num)
        { return (head += num) - num; }
    
    /*
     * Reverse a previous get() operation, making the last 'num' bytes read
     * available for a subsequent get().
     */
    void unget(size_t num)
        { head -= num; }
    
    /*
     * allocate the next 'num' bytes of the minibuffer (presumably for
     * writing)
     */
    unsigned char *alloc(size_t num)
        { return (tail += num) - num; }

    /*
     * Reverse a previous alloc() operation, making the last 'num' bytes
     * allocated available for a subsequent alloc() or put().
     */
    void unalloc(size_t num)
        { tail -= num; }
    
    /*
     * copy the given data into the next 'num' bytes of the minibuffer.
     */
    void put(const void *data, size_t num)
        { memcpy(alloc(num), data, num); }
    
    /*
     * return the number of bytes that must be retrieved with get() in order
     * to find the first instance of 'ch'.  A return value of 0 means that
     * there is no 'ch' in the minibuffer.
     */
    size_t strchr(unsigned char ch) const;
    size_t strchr(char ch) const
	{ return strchr((unsigned char)ch); }
    
    /*
     * Count the number of leading bytes that match any in chlist.
     * If reverse==true, match bytes that are _not_ in chlist.
     */
    size_t match(const unsigned char chlist[], size_t numch,
		   bool reverse = false) const;
    size_t match(const char chlist[], bool reverse = false) const
        { return match((const unsigned char *)chlist,
			 strlen(chlist), reverse); }
};


DeclareWvList(WvMiniBuffer);


class WvBuffer
{
    WvMiniBufferList list;
    size_t inuse;
    
public:
    WvBuffer()
        { inuse = 0; }
    
    size_t used() const
        { return inuse; }

    /*
     * Clear the entire buffer.
     */
    void zap();
    
    /*
     * Return the next 'num' bytes in the buffer.  Pointer is valid until
     * next zap() or get().  Returns NULL if there are not enough bytes
     * in the buffer.
     */
    unsigned char *get(size_t num);
    
    /*
     * Undo all or part of the previous get().  You can unget() up to the
     * number of bytes you did in the last get(), assuming you have not done
     * any other buffer operations in the meantime.
     */
    void unget(size_t num);
    
    /*
     * allocate 'num' bytes in the buffer and return a pointer to its start.
     * Pointer is valid until next zap() or get().
     */
    unsigned char *alloc(size_t num);

    /*
     * unallocate the last 'num' bytes in the buffer that were previously
     * allocated using alloc() or put().  They are then available for a
     * subsequent alloc() or put().
     */
    void unalloc(size_t num);
    
    /* 
     * copy 'buf' into the next 'num' bytes of buffer.
     */
    void put(const void *buf, size_t num);
    
    /*
     * copy a WvString into the buffer, not including the terminating nul.
     */
    void put(WvStringParm str);
    
    /*
     * add a single character to the buffer.
     */
    void putch(int ch)
        { *alloc(1) = ch; }
    
    /*
     * _move_ (not copy) the contents of another WvBuffer into this buffer.
     * This is done by physically taking the WvMiniBuffer objects from one
     * buffer and adding them at the end of this one.
     */
    void merge(WvBuffer &buf);
    
    /*
     * Return the entire buffer as a nul-terminated WvString.  If the buffer
     * contains nul characters, they'll seem to terinate the string.
     */
    WvString getstr();

    /*
     * return the number of bytes that would have to be read to find the
     * first character 'ch', or zero if 'ch' is not in the buffer.
     */
    size_t strchr(unsigned char ch);
    size_t strchr(char ch)
        { return strchr((unsigned char)ch); }

    /*
     * return the number of leading bytes that match any in chlist.
     * If reverse==true, match bytes that are NOT in chlist.
     */
    size_t match(const unsigned char chlist[], size_t numch,
		   bool reverse = false);
    size_t match(const char chlist[], bool reverse = false)
        { return match((const unsigned char *)chlist, strlen(chlist),
			 reverse); }
    
    int num_of_bufs()
        { return list.count(); }
};


#endif // __WVBUFFER_H
