/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A linked list container.
 */
#ifndef __WVLINKLIST_H
#define __WVLINKLIST_H

#include <assert.h>
#include "wvsorter.h"

/**
 * The untyped base class of WvList<T>.
 * <p>
 * Putting common code in here allows us to prevent it from being
 * replicated by each template instantiation of WvList<T>.
 * </p>
 */
class WvListBase
{
public:
    WvLink head, *tail;
    WvListBase() : head(NULL, false)
        { tail = &head; }
    WvListBase(const WvListBase &l); // copy constructor - not actually defined anywhere!
    WvListBase& operator= (const WvListBase &l);
    size_t count() const;

    // this could be done with count() but it would be slow
    bool isempty() const
        { return head.next == NULL; }

    // the base class for list iterators
    class IterBase
    {
    public:
	const WvListBase *list;
	WvLink *link, *prev;

	IterBase(const WvListBase &l)
            { list = &l; link = NULL; }
	void rewind() // dropping a const pointer here!  Danger!
            { prev = NULL; link = &((WvListBase *)list)->head; }
	WvLink *next()
            { prev = link; return link = link->next; }
	WvLink *cur() const
            { return link; }

	// set 'cur' to the WvLink that points to 'data', and return the
	// link.  If 'data' is not found, sets cur=NULL and returns NULL.
	WvLink *find(const void *data);
    };
};

/**
 * A linked list container class.
 * <p>
 * Some rather horrible macros are used to declare actual concrete
 * list types.
 * </p><p>
 * Example:
 * <pre>
 *   DeclareWvList(WvString);
 *
 *   int main()
 *   {
 *       WvStringList l;
 *       WvStringList::Iter i(l);
 *
 *       ... fill the list ...
 *
 *       i.rewind();
 *       while (i.next())
 *           printf("%s\\n", i.str);
 *   }
 * </pre>
 * </p><p>
 * Deallocating list will free all of the WvLinks in the list, but
 * will only free the user objects that were added with auto_free
 * set to true.
 * </p><p>
 * We need to malloc memory for each WvLink as well as the data it
 * stores; this is unnecessarily slow.  I would rather have made a
 * base "Link" class for object types that could be stored as links
 * in a list, and then used object->next instead of all the
 * List Iterator stuff, but the end result was pure ugliness, so I
 * gave up.  At least this way, the same object can be in multiple
 * lists.
 * </p><p>
 * List type construction is facilitated by the following macros:
 * <ul>
 * <li>DeclareWvList(Type): creates a subclass named WvListType
 *     that contains pointers to Type.</li>
 * <li>DeclareWvList2(Type, code...): as the above, but includes the
 *     specified block of code into the public section of the
 *     class declaration.
 *     eg. DeclareWvList2(WvString, void autofill(););</li>
 * <li>DeclareWvList3(Type, name, code...): as the above, but
 *     calls the resulting class by the specified name.</li>
 * </ul>
 * </p>
 * @param T the object type
 */
template<class T>
class WvList : public WvListBase
{
public:
    WvList()
	{ }
	
    ~WvList()
	{ zap(); }
	
    // default implementations
    void setup() {}
    void shutdown() {}

    void zap()
    {
        while (head.next)
            unlink_after(& head);
    }

    T *first() const
        {
	    assert(!isempty());
	    return (T*)head.next->data;
	}

    T *last() const
        { return (T*)tail->data; }

    void add_after(WvLink *after, T *data, bool auto_free,
			char *id = NULL )
        { (void)new WvLink((void *)data, after, tail, auto_free, id); }

    void append(T *data, bool auto_free, char *id = NULL)
	{ add_after(tail, data, auto_free, id); }

    inline void add(T *data, bool auto_free, char *id = NULL)
        { append(data, auto_free, id); }

    void prepend(T *data, bool auto_free, char *id = NULL)
	{ add_after(&head, data, auto_free, id); }

    void unlink(T *data)
        { Iter i(*this); while (i.find(data)) i.unlink(); }

    void unlink_first()
        { Iter i(*this); i.rewind(); i.next(); i.unlink(); }

    void unlink_after(WvLink *after)
    {
        WvLink *next = after->next;
        T *obj = next->auto_free ?
            static_cast<T*>(next->data) : NULL;
        if (next == tail) tail = after;
        next->unlink(after);
        delete obj;
    }

    class Iter : public WvListBase::IterBase
    {
    public:
        Iter(const WvList &l) : IterBase(l)
            { }
        T *ptr() const
            { return (T *)link->data; }
	WvIterStuff(T);
	
        void unlink()
        {
	    if (prev) ((WvList *)list)->unlink_after(prev);
	    link = prev->next;
        }
	
	// like unlink(), except it leaves the iterator semi-invalid so
	// you have to do a next() before accessing it again.  In retrospect,
	// this is probably a nicer API than unlink() (since you can then
	// just do next() at the top of the loop regardless of whether you
	// did unlink()) but it would be too painful to change it everywhere
	// right now.
	void xunlink()
	{
	    if (prev) ((WvList *)list)->unlink_after(prev);
	    link = prev;
	}
    };
    
    typedef class WvSorter<T, WvListBase, WvListBase::IterBase> Sorter;
};


#define DeclareWvList3(_type_,_newname_,_extra_)      	\
    class _newname_ : public WvList<_type_> 		\
    { 							\
    public: 						\
        _newname_() { setup(); }			\
        						\
        ~_newname_() { shutdown(); }			\
	_extra_ 					\
    };

#define DeclareWvList2(_type_,_extra_)  		\
		DeclareWvList3(_type_,_type_##List,_extra_ )

#define DeclareWvList(_type_) DeclareWvList2(_type_,;)


#endif // __WVLINKLIST_H
