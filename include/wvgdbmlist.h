/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A linked list container backed by a gdbm database.
 */
#ifndef __WVGDBMLIST_H
#define __WVGDBMLIST_H

#include "wvgdbmhash.h"

/**
 * A class based on WvGdbmHash that lets you store WvBufs and auto-assign
 * them Index values as keys.  This is convenient for implementing various
 * data structures in the on-disk hash, since you can use Index values
 * wherever an in-memory structure would use a pointer.
 * 
 * NOTE: Index values <= 0 have a special meaning, and will never be
 * assigned automatically.  WvGdbmAlloc uses Index # -1 itself as the
 * beginning of the FREELIST.  The others you can use as you wish.
 */
class WvGdbmAlloc
{
public:
    enum { FREELIST = -1 };
    
    typedef int32_t Index;
    typedef WvGdbmHash<Index, WvBuf> LinkHash;
    
    LinkHash hash;
    
    WvGdbmAlloc(WvStringParm filename) : hash(filename)
        { }
    
private:
    Index next(Index i)
    {
	WvBuf *buf = &hash[i];
	Index next = wv_deserialize<Index>(*buf);
	return next;
    }
    
    void link(Index prev, Index next)
    {
	unsigned char junk[16];
	WvInPlaceBuf buf(junk, 0, 16);
	wv_serialize(buf, next);
	hash.add(prev, buf, true);
    }
    
public:
    void zap()
    {
	hash.zap();
	link(FREELIST, 1);
    }

    Index alloc()
    {
	Index i = next(FREELIST);
	if (!hash.exists(i))
	{
	    // this is the highest allocated value.  Preallocate the
	    // next one so we don't lose our place.
	    assert(!hash.exists(i+1));
	    link(FREELIST, i+1);
	}
	else
	    link(FREELIST, next(i));
	return i;
    }
    
    void unalloc(Index i)
    {
	link(i, next(FREELIST));
	link(FREELIST, i);
    }
};


/**
 * A class similar to WvList, but storing its values on disk in a WvGdbmHash.
 * 
 * FIXME: I have no idea if this is fast, slow, stupid, or ingenious.  I
 * suspect it's probably quite inefficient - doing it entirely without gdbm
 * and writing our own space allocator would probably make more sense.
 * 
 * FIXME: we should use a common non-templated base class rather than
 * implementing everything inline.
 * 
 * FIXME: if HEAD and TAIL weren't hardcoded, we could put more than one
 * list in the same WvGdbmHash.  This would probably be pretty useful.
 */
template <typename T>
class WvGdbmList
{
    typedef WvGdbmAlloc::Index Index;
    WvGdbmAlloc alloc;
    WvGdbmAlloc::LinkHash &hash;
    
public:
    class Iter;
    friend class WvGdbmList::Iter;
    
    enum { HEAD = 0, TAIL = -1000 };
    
    struct Link
    {
	Index next;
	WvBuf *buf;
    private:
	T *_data;
	
    public:
	Link()
	    { _data = NULL; buf = NULL; }
	~Link()
	    { zap(); }
	
	T *data()
	{
	    if (buf && !_data)
	    {
		if (buf->used())
		    _data = wv_deserialize<T *>(*buf);
		else
		    _data = NULL;
		buf = NULL;
	    }
	    return _data;
	}
	
	void zap()
	    { if (_data) delete _data; _data = NULL; }
    } saved;
    
    Index retrieve(Index i)
    {
	saved.zap();
	saved.buf = &hash[i];
	saved.next = wv_deserialize<Index>(*saved.buf);
	//printf(" ret %d/%d (%d left)\n", i, saved.next, saved.buf->used());
	return saved.next;
    }
    
    void save(Index i, Index next, const T *data)
    {
	WvDynBuf buf;
	wv_serialize(buf, next);
	if (data)
	    wv_serialize(buf, *data);
	//printf("save %d/%d/%p (%d bytes)\n", i, next, data, buf.used());
	hash.add(i, buf, true);
    }

public:    
    WvGdbmList(WvStringParm filename) : alloc(filename), hash(alloc.hash)
    { 
	init();
    }
    
    void init()
    {
	if (!hash.exists(HEAD) || !hash.exists(TAIL))
	{
	    // corrupted or newly created!
	    zap();
	}
    }
    
    void zap()
    {
	alloc.zap();
	save(HEAD, HEAD, NULL);
	save(TAIL, HEAD, NULL);
	assert(!hash.exists(1));
    }
    
    size_t count()
    {
	int count = 0;
	for (int next = retrieve(HEAD); next != HEAD; next = retrieve(next))
	    count++;
	return count;
    }
    
    bool isempty()
    {
	return retrieve(HEAD) == HEAD;
    }
    
    T *first()
    {
	// HEAD is not an actual element - it just points to the first one
	retrieve(retrieve(HEAD));
	return saved.data();
    }
    
    T *last()
    {
	// TAIL is not an actual element - it just points to the last one
	// (and nobody links to TAIL)
	retrieve(retrieve(TAIL)); 
	return saved.data();
    }
    
    void add_after(Index after, const T *data, bool auto_free = false,
		   void *id = NULL)
    {
	Index i = alloc.alloc();
	retrieve(after);
	Index next = retrieve(after);
	save(i, next, data);
	save(after, i, saved.data());
	
	if (next == HEAD)
	    save(TAIL, i, NULL);
	if (auto_free)
	    delete data; // already done with it!
    }
    
    void append(const T &data, bool auto_free = false, void *id = NULL)
	{ add_after(retrieve(TAIL), &data, auto_free, id); }
    void prepend(const T &data, bool auto_free = false, void *id = NULL)
        { add_after(HEAD, &data, auto_free, id); }

    void append(const T *data, bool auto_free = false, void *id = NULL)
	{ add_after(retrieve(TAIL), data, auto_free, id); }
    void prepend(const T *data, bool auto_free = false, void *id = NULL)
        { add_after(HEAD, data, auto_free, id); }

private:
    // this works in a WvList, but it's kind of hard in a GdbmList.  So we
    // won't implement it.
    void unlink(T *data);

public:
    void unlink_first()
        { unlink_after(HEAD); }
    
    void unlink_after(Index prev)
    {
	Index cur = retrieve(prev);
	Index next = retrieve(cur);
	if (next == HEAD)
	    save(TAIL, prev, NULL);
	retrieve(prev);
	save(prev, next, saved.data());
	// hash.remove(cur); // unalloc replaces this
	alloc.unalloc(cur);
    }
    
    class Iter
    {
	typedef WvGdbmList::Index Index;
    public:
	WvGdbmList &list;
	Index prev, xcur, xnext;
	
	Iter(WvGdbmList &_list) : list(_list)
	    { }
	
	void rewind()
	    { prev = HEAD; xcur = HEAD; xnext = list.retrieve(xcur); }
	bool cur()
	    { return xcur != HEAD; }
	bool next()
	    { prev = xcur; xcur = xnext; xnext = list.retrieve(xcur);
		return cur(); }

	/**
         * Unlinks the current element from the list like in WvList.
	 * You usually want xunlink() instead.
         */
	void unlink()
	    { list.unlink_after(prev); xcur = list.retrieve(prev); 
	      xnext = list.retrieve(xcur); }
	
	/**
         * Unlinks the current element from the list like in WvList.
	 * The iterator becomes invalid until next(), but next() does
	 * exactly what it would have done if you hadn't done xunlink().
	 * See WvLink::Iter::xunlink() for the reasoning here.
         */
	void xunlink()
	    { list.unlink_after(prev); xcur = prev; }
	
	T *ptr() const
	    { return list.saved.data(); }
	WvIterStuff(T);
    };
};


// DeclareWvList-compatible macro for people who want to hackily see what
// happens if they replace their WvList with a WvGdbmList.
#define DeclareWvGdbmList(__type__) \
    class __type__##List : public WvGdbmList<__type__> \
    { \
    public: \
	__type__##List() : WvGdbmList<__type__>((srand(time(NULL)), \
						     random())) {} \
    }


#endif // __WVGDBMLIST_H
