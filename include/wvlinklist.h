/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Implementation of a Linked List management class, or rather, macros that
 * declare arbitrary linked list management classes.  This looks pretty
 * horrible, but lets us do stuff like this:
 *
 *       DeclareWvList(WvString);
 *
 *       int main()
 *       {
 *           WvStringList l;
 *           WvStringList::Iter i(l);
 *           ... fill the list ...
 *
 *           i.rewind();
 *           while (i.next())
 *               printf("%s\n", i.str);
 *
 *           ...
 *       }
 *
 * NOTES:
 *    - We need to malloc memory for each WvLink as well as the data it
 *        stores; this is unnecessarily slow.  I would rather have made a
 *        base "Link" class for object types that could be stored as links
 *        in a list, and then used object->next instead of all the
 *        List Iterator stuff, but the end result was pure ugliness, so I
 *        gave up.  At least this way, the same object can be in multiple
 *        lists.
 *
 *    - DeclareWvList2() allows the user to include arbitrary lines
 *        inside the class definition.  For example:
 *             DeclareWvList2(WvString, void autofill(););
 *
 *    - Deallocating a *List object will free all the WvLinks in the list,
 *        but not necessarily all the objects that the WvLinks point to
 *        (depending on the value of link->auto_free)
 */
#ifndef __WVLINKLIST_H
#define __WVLINKLIST_H

#include <assert.h>
#include "wvsorter.h"

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

template <class _type_>
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
	WvLink *l, *n=head.next;
	head.next = NULL;
	tail = &head;
	while ((l = n) != NULL)
	{
	    n = l->next;
            if (l->auto_free) delete (_type_ *)l->data;
	    delete l;
	}
    }

    _type_ *first() const
        {
	    assert(!isempty());
	    return (_type_*)head.next->data;
	}

    _type_ *last() const
        { return (_type_*)tail->data; }

    void add_after(WvLink *after, _type_ *data, bool auto_free,
			char *id = NULL )
        { (void)new WvLink((void *)data, after, tail, auto_free, id); }

    void append(_type_ *data, bool auto_free, char *id = NULL)
	{ add_after(tail, data, auto_free, id); }

    inline void add(_type_ *data, bool auto_free, char *id = NULL)
        { append(data, auto_free, id); }

    void prepend(_type_ *data, bool auto_free, char *id = NULL)
	{ add_after(&head, data, auto_free, id); }

    void unlink(_type_ *data)
        { Iter i(*this); while (i.find(data)) i.unlink(); }

    void unlink_first()
        { Iter i(*this); i.rewind(); i.next(); i.unlink(); }

    void unlink_after(WvLink *after)
    {
         if (after->next->auto_free)
             delete (_type_ *)after->next->data;
         if (after->next == tail) tail = after;
         if (after->next) after->next->unlink(after);
    }

    class Iter : public WvListBase::IterBase
    {
    public:
        Iter(const WvList &l) : IterBase(l)
            { }
        _type_ *ptr() const
            { return (_type_ *)link->data; }
	WvIterStuff(_type_);
	
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
    
    typedef class WvSorter<_type_,WvListBase,WvListBase::IterBase> Sorter;
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
