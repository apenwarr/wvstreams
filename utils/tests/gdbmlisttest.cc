#include "wvgdbmhash.h"

/**
 * A class similar to WvList, but storing its values on disk in a WvGdbmHash.
 * 
 * FIXME: I have no idea if this is fast, slow, stupid, or ingenious.
 * FIXME: we should use a common non-templated base class rather than
 * implementing everything inline.
 */
template <typename T>
class WvGdbmList
{
public:
    class Iter;
    
    /// NOTE: HEAD must == false!
    enum { HEAD = 0, TAIL = -1, FREELIST = -2 };
    
    friend class WvGdbmList::Iter;
    
    typedef int32_t Index;
    typedef WvGdbmHash<Index, WvBuf> LinkHash;
    
    LinkHash hash;
    
    struct Link
    {
	Index next;
	WvBuf *buf;
    private:
	T *data;
	
    public:
	Link()
	    { data = NULL; buf = NULL; }
	~Link()
	    { zap(); }
	
	T *get()
	{
	    if (buf)
	    {
		if (buf->used())
		    data = wv_deserialize<T *>(*buf);
		else
		    data = NULL;
		buf = NULL;
	    }
	    return data;
	}
	
	void zap()
	    { if (data) delete data; data = NULL; }
    };
    
    Link saved;
    
    Index retrieve(Index i)
    {
	saved.zap();
	saved.buf = &hash[i];
	saved.next = wv_deserialize<Index>(*saved.buf);
	/*printf(" ret %d/%d (%d bytes left)\n",
	       i, saved.next, saved.buf->used()); */
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

    Index alloc()
    {
	Index i = retrieve(FREELIST);
	if (!hash.exists(i))
	{
	    // this is the highest allocated value.
	    assert(!hash.exists(i+1));
	    save(FREELIST, i+1, NULL);
	}
	else
	    save(FREELIST, retrieve(i), NULL);
	return i;
    }
    
    void unalloc(Index i)
    {
	save(i, retrieve(FREELIST), NULL);
	save(FREELIST, i, NULL);
    }
    
public:    
    WvGdbmList(WvStringParm filename) : hash(filename)
    { 
	if (!hash.exists(HEAD) || !hash.exists(TAIL))
	{
	    // corrupted or newly created!
	    zap();
	}
    }
    
    void zap()
    {
	hash.zap();
	save(HEAD, HEAD, NULL);
	save(TAIL, HEAD, NULL);
	save(FREELIST, 1, NULL);
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
	return saved.get();
    }
    
    T *last()
    {
	// TAIL is not an actual element - it just points to the last one
	// (and nobody links to TAIL)
	retrieve(retrieve(TAIL)); 
	return saved.get();
    }
    
    void add_after(Index after, const T *data)
    {
	Index i = alloc();
	retrieve(after);
	Index next = retrieve(after);
	save(i, next, data);
	save(after, i, saved.get());
	
	if (next == HEAD)
	    save(TAIL, i, NULL);
    }
    
    void append(const T &data)
	{ add_after(retrieve(TAIL), &data); }
    void prepend(const T &data)
        { add_after(HEAD, &data); }
    
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
	save(prev, next, saved.get());
	// hash.remove(cur); // unalloc replaces this
	unalloc(cur);
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
	Index cur()
	    { return xcur; }
	Index next()
	    { prev = xcur; xcur = xnext; xnext = list.retrieve(xcur);
		return xcur; }

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
	    { return list.saved.get(); }
	WvIterStuff(T);
    };
};


int main()
{
    WvGdbmList<WvString> list("dbmlist");
    list.zap();
    printf("empty:%d\n", list.isempty());
    printf("count:%d\n", list.count());
    list.append("honky 1");
    list.append("honky 2");
    list.append("honky 3");
    list.append("honky 4");
    list.append("honky 5");
    list.append("honky 6");
    list.append("honky 77");
    list.prepend("honky 0");
    printf("empty:%d\n", list.isempty());
    printf("count:%d\n", list.count());
    printf("'%s'\n", list.first()->cstr());
    
    bool odd = false;
    printf("\nIterator test:\n");
    WvGdbmList<WvString>::Iter i(list);
    for (i.rewind(); i.next(); )
    {
	printf("    %5d = ", i.cur());
	fflush(stdout);
	printf("'%s'\n", i->cstr());
	if (odd)
	    i.xunlink();
	odd = !odd;
    }
    
    list.unlink_first();
    
    printf("\nIterator test:\n");
    for (i.rewind(); i.next(); )
    {
	printf("    %5d = ", i.cur());
	fflush(stdout);
	printf("'%s'\n", i->cstr());
    }
    
    return 0;
}
