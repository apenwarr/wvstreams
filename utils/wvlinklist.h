/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
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
 *    - g++ does templates badly, or we would use those.
 * 
 *    - We need to malloc memory for each WvLink as well as the data it
 *        stores; this is unnecessarily slow.  I would rather have made a
 *        base "Link" class for object types that could be stored as links
 *        in a list, and then used object->next instead of all the
 *        List Iterator stuff, but the end result was pure ugliness, so I
 *        gave up.  At least this way, the same object can be in multiple
 *        lists.
 * 
 *    - DeclareWvList() will work both inside a function and outside all
 *        functions (ie. local or global scope).  However, for some strange
 *        reason g++ (2.7.2.1) refuses to inline the functions if
 *        the list is declared locally, resulting in massive wastage.  So,
 *        always call DeclareWvList() in a global scope.
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


class WvList
{
public:
    WvLink head, *tail;
    WvList() : head(NULL, false)
        { tail = &head; }
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
	WvList *list;
	WvLink *link, *prev;
	
	IterBase(WvList &l)
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
};


#define DeclareWvList3(_type_,_newname_,_extra_)      	\
class _newname_ : public WvList				\
{ 							\
public: 						\
    _newname_()						\
	{ setup(); }					\
							\
    ~##_newname_() 					\
	{ shutdown(); _zap(); }				\
							\
    void zap()						\
	{ _zap(); head.next = NULL; tail = &head; }	\
							\
    void _zap()						\
    {							\
	WvLink *l, *n=head.next; 			\
	while ((l = n) != NULL) 			\
	{ 						\
	    n = l->next; 				\
            if (l->auto_free) delete (_type_ *)l->data; \
	    delete l; 					\
	} 						\
    }							\
							\
    void add_after(WvLink *after, _type_ *data, bool auto_free, \
			char *id = NULL )		\
        { (void)new WvLink((void *)data, after, tail, auto_free, id); }	\
							\
    void append(_type_ *data, bool auto_free, char *id = NULL) \
	{ add_after(tail, data, auto_free, id); }	\
							\
    void prepend(_type_ *data, bool auto_free, char *id = NULL) \
	{ add_after(&head, data, auto_free, id); }	\
							\
    void unlink(_type_ *data)				\
        { Iter i(*this); while (i.find(data)) i.unlink(); }\
							\
    void unlink_after(WvLink *after) 			\
    { 							\
         if (after->next->auto_free)			\
             delete (_type_ *)after->next->data;	\
         if (after->next == tail) tail = after; 	\
         if (after->next) after->next->unlink(after); 	\
    } 							\
							\
    class Iter : public IterBase			\
    { 							\
    public: 						\
        Iter(_newname_ &l) : IterBase(l)		\
            { } 					\
        _type_ &data() const 				\
            { return *(_type_ *)link->data; } 		\
	operator _type_& () const			\
	    { return data(); }				\
	_type_ &operator () () const			\
	     { return data(); }				\
        void unlink() 					\
        {						\
	    if (prev) ((_newname_ *)list)->unlink_after(prev); \
	    link = prev->next;				\
        }						\
    };							\
							\
public: 						\
    _extra_ 						\
};

#define DeclareWvList2(_type_,_extra_) \
		DeclareWvList3(_type_,_type_##List,_extra_ )
#define DeclareWvList(_type_) DeclareWvList2(_type_, )

#endif // __WVLINKLIST_H
