/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
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

#include <stdlib.h>  // for 'NULL'


#define WvIterStuff(_type_) \
	operator _type_& () const \
	    { return *ptr(); } \
	_type_ &operator () () const \
	    { return *ptr(); } \
	_type_ *operator -> () const \
	    { return ptr(); } \
        _type_ &operator* () const \
            { return *ptr(); }
	


// note: auto_free behaviour is a little bit weird; since WvLink does not
// know what data type it has received, there is no way it can call the
// right destructor.  So, the WvList needs to handle the data deletion
// by itself.  On the other hand, the auto_free flag needs to be stored in
// the WvLink.  <sigh>...
//
class WvLink
{
public:
    void *data;
    WvLink *next;
    char *id;
    unsigned auto_free : 1;

    WvLink(void *_data, bool _auto_free, char *_id = NULL)
        { data = _data; next = NULL; auto_free = (unsigned)_auto_free;
	    id = _id; }

    WvLink(void *_data, WvLink *prev, WvLink *&tail, bool _auto_free,
	   char *_id = NULL);

    void unlink(WvLink *prev)
    {
	prev->next = next;
	delete this;
    }
};


class WvListBase
{
public:
    WvLink head, *tail;
    WvListBase() : head(NULL, false)
        { tail = &head; }
    WvListBase(const WvListBase &l); // copy constructor - not actually defined anywhere!
    WvListBase& operator= (const WvListBase &l);
    void setup()
        { /* default: do nothing */ }
    void shutdown()
        { /* default: do nothing */ }
    size_t count() const;

    // this could be done with count() but it would be slow
    bool isempty() const
        { return head.next == NULL; }

    // the base class for list iterators
    class IterBase
    {
    public:
	WvListBase *list;
	WvLink *link, *prev;

	IterBase(WvListBase &l)
            { list = &l; link = NULL; }
	void rewind()
            { prev = NULL; link = &list->head; }
	WvLink *next()
            { prev = link; return link = link->next; }
	WvLink *cur() const
            { return link; }

	// set 'cur' to the WvLink that points to 'data', and return the
	// link.  If 'data' is not found, sets cur=NULL and returns NULL.
	WvLink *find(const void *data);
    };

    // the base class for sorted list iterators.
    // It is similar to IterBase, except for rewind(), next(), and cur().
    // The sorting is done in rewind(), which makes an array of WvLink
    // pointers and calls qsort.  "lptr" is a pointer to the current WvLink *
    // in the array, and next() increments to the next one.
    // NOTE: we do not keep "prev" because it makes no sense to do so.
    //       I guess Sorter::unlink() will be slow... <sigh>
    class SorterBase
    {
    public:
	typedef int (CompareFunc)(const void *a, const void *b);
	    
        WvListBase *list;
        WvLink **array;
        WvLink **lptr;

        SorterBase(WvListBase &l)
            { list = &l; array = lptr = NULL; }
        virtual ~SorterBase()
            { if (array) delete array; }
        WvLink *next()
            { return lptr ? *(++lptr)
                          : *(lptr = array); }
        WvLink *cur() const
            { return lptr ? *lptr : &list->head; }
    protected:
        void rewind(CompareFunc *cmp);
    };
};

template <class _type_>
class WvList : public WvListBase
{
public:
    WvList()
	{ setup(); }

    ~WvList()
	{ shutdown(); zap(); }

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

    void add_after(WvLink *after, _type_ *data, bool auto_free,
			char *id = NULL )
        { (void)new WvLink((void *)data, after, tail, auto_free, id); }

    void append(_type_ *data, bool auto_free, char *id = NULL)
	{ add_after(tail, data, auto_free, id); }

    void prepend(_type_ *data, bool auto_free, char *id = NULL)
	{ add_after(&head, data, auto_free, id); }

    void unlink(_type_ *data)
        { Iter i(*this); while (i.find(data)) i.unlink(); }

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
        Iter(WvList &l) : IterBase(l)
            { }
        _type_ *ptr() const
            { return (_type_ *)link->data; }
	WvIterStuff(_type_);
	
        void unlink()
        {
	    if (prev) ((WvList *)list)->unlink_after(prev);
	    link = prev->next;
        }
    };

    class Sorter : public WvListBase::SorterBase
    {
    public:
	typedef int (RealCompareFunc)(const _type_ *a, const _type_ *b);
	RealCompareFunc *cmp;

        Sorter(WvList &l, RealCompareFunc *_cmp)
            : SorterBase(l), cmp(_cmp)
            { }
        _type_ *ptr() const
            { return (_type_ *)(*lptr)->data; }
	WvIterStuff(_type_);
        void unlink()
        {
            ((WvList *)list)->unlink(ptr());
            lptr += sizeof(WvLink *);
        }
        void rewind()
            { SorterBase::rewind((CompareFunc *)cmp); }
    };
};


#define DeclareWvList3(_type_,_newname_,_extra_)      	\
    class _newname_ : public WvList<_type_> 		\
    { 							\
    public: 						\
	_extra_ 					\
    };

#define DeclareWvList2(_type_,_extra_)  		\
		DeclareWvList3(_type_,_type_##List,_extra_ )

#define DeclareWvList(_type_) DeclareWvList2(_type_, )


#endif // __WVLINKLIST_H
