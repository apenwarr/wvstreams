/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A linked list container.
 */
#ifndef __WVLINKLIST_H
#define __WVLINKLIST_H

#include <assert.h>
#include "wvsorter.h"

/**
 * @internal
 * The untyped base class of WvList<T>.
 * <p>
 * Putting common code in here allows us to prevent it from being
 * replicated by each template instantiation of WvList<T>.
 * </p>
 */
class WvListBase
{
protected:
    WvListBase(const WvListBase &l); // copy constructor - not actually defined anywhere!
    WvListBase& operator= (const WvListBase &l);
    
public:
    WvLink head, *tail;

    /**
     * Creates an empty linked list.
     */
    WvListBase() : head(NULL, false)
        { tail = &head; }

    /**
     * Returns the number of elements in the list.
     * <p>
     * This function causes a full traversal of the list which may be
     * overly inefficient depending on how and when it is used.
     * </p>
     * @return the number of elements
     */
    size_t count() const;

    /**
     * Quickly determines if the list is empty.
     * <p>
     * This is much faster than checking count() == 0.
     * </p>
     * @return true if empty
     */
    bool isempty() const
        { return head.next == NULL; }

    /**
     * @internal
     * The untyped base class of WvList<T>::Iter.
     * <p>
     * Putting common code in here allows us to prevent it from being
     * replicated by each template instantiation of WvList<T>.
     * </p>
     */
    class IterBase
    {
    public:
	const WvListBase *list;
	WvLink *link, *prev;

        /**
         * Binds the iterator to the specified list.
         * @param l the list
         */
	IterBase(const WvListBase &l)
            { list = &l; link = NULL; }

        /**
         * Rewinds the iterator to make it point to an imaginary element
         * preceeding the first element of the list.
         */
	void rewind() // dropping a const pointer here!  Danger!
            { prev = NULL; link = &((WvListBase *)list)->head; }

        /**
         * Moves the iterator along the list to point to the next element.
         * <p>
         * If the iterator had just been rewound, it now points to the
         * first element of the list.
         * </p>
         * @return the current WvLink pointer, or null if there were no
         *         more elements remaining in the traversal sequence
         */
	WvLink *next()
            { prev = link; return link = link->next; }

        /**
         * Returns a pointer to the WvLink at the iterator's current location.
         * @return the current WvLink pointer, or null if there were no
         *         more elements remaining in the traversal sequence
         */
	WvLink *cur() const
            { return link; }

        /**
         * Rewinds the iterator and repositions it over the element that
         * matches the specified value.
         * <p>
         * Uses pointer equality (object identity) as the criteria for
         * finding the matching element.
         * </p><p>
         * It is not possible to use find(const void*) to locate multiple
         * matching elements unless the list is altered between invocations
         * since it always starts searching from the head of the list
         * rather than from the current location.
         * </p>
         * @return the current WvLink pointer, or null if no such element
         *         was found
         */
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
    /**
     * Creates an empty linked list.
     */
    WvList()
	{ }
    
    /**
     * Destroys the linked list.
     * <p>
     * Destroys any elements that were added with auto_free == true.
     * </p>
     */
    ~WvList()
	{ zap(); }
	
    /**
     * Invoked by subclasses after the linked list is first created.
     */
    void setup() {}
    
    /**
     * Invoked by subclasses before the linked list is destroyed.
     */
    void shutdown() {}

    /**
     * Clears the linked list.
     * <p>
     * Destroys any elements that were added with auto_free == true.
     * </p>
     */
    void zap()
    {
        while (head.next)
            unlink_after(& head);
    }

    /**
     * Returns a pointer to the first element in the linked list.
     * <p>
     * The list must be non-empty.
     * </p>
     * @return the element pointer, possibly null
     */
    T *first() const
        {
	    assert(!isempty());
	    return (T*)head.next->data;
	}

    /**
     * Returns a pointer to the last element in the linked list.
     * <p>
     * The list must be non-empty.
     * </p>
     * @return the element pointer, possibly null
     */
    T *last() const
        { return (T*)tail->data; }

    /**
     * Adds the element after the specified link in the list.
     *
     * @param link the link preceeding the desired location of the element
     *             to be inserted, non-null
     * @param data the element pointer, may be null
     * @param auto_free if true, takes ownership of the element
     * @param id an optional string to associate with the element, or null
     */
    void add_after(WvLink *after, T *data, bool auto_free,
			char *id = NULL )
        { (void)new WvLink((void *)data, after, tail, auto_free, id); }

    /**
     * Appends the element to the end of the list.
     *
     * @param data the element pointer, may be null
     * @param auto_free if true, takes ownership of the element
     * @param id an optional string to associate with the element, or null
     */
    void append(T *data, bool auto_free, char *id = NULL)
	{ add_after(tail, data, auto_free, id); }

    /**
     * Synonym for append(T*, bool, char*).
     * @see append(T*, bool, char*)
     */
    inline void add(T *data, bool auto_free, char *id = NULL)
        { append(data, auto_free, id); }

    /**
     * Prepends the element to the beginning of the list.
     *
     * @param data the element pointer, may be null
     * @param auto_free if true, takes ownership of the element
     * @param id an optional string to associate with the element, or null
     */
    void prepend(T *data, bool auto_free, char *id = NULL)
	{ add_after(&head, data, auto_free, id); }

    /**
     * Unlinks the specified element from the list.
     * <p>
     * Destroys the element if it was added with auto_free == true.
     * </p>
     * @param data the element pointer, may be null
     */
    void unlink(T *data)
        { Iter i(*this); while (i.find(data)) i.unlink(); }

    /**
     * Unlinks the first element from the list.
     * <p>
     * Destroys the element if it was added with auto_free == true.
     * </p>
     */ 
    void unlink_first()
        { Iter i(*this); i.rewind(); i.next(); i.unlink(); }

    /**
     * Unlinks the element that follows the specified link in the list.
     * <p>
     * Destroys the element if it was added with auto_free == true.
     * </p>
     * @param after the link preceeding the element to be removed, non-null
     */ 
    void unlink_after(WvLink *after)
    {
        WvLink *next = after->next;
        T *obj = next->auto_free ?
            static_cast<T*>(next->data) : NULL;
        if (next == tail) tail = after;
        next->unlink(after);
        delete obj;
    }

    /**
     * The iterator type for linked lists.
     * <p>
     * An iterator instance does not initially point to any valid
     * elements in a list.  Before using, it must be reset using rewind()
     * which causes it to point to an imaginary element located before
     * the first elements in the list.  Then next() must be invoked
     * to incrementally move the iterator along the list to first element,
     * and then later to the second, third, and subsequent elements.
     * </p>
     */
    class Iter : public WvListBase::IterBase
    {
    public:
        /**
         * Binds the iterator to the specified list.
         * @param l the list
         */
        Iter(const WvList &l) : IterBase(l)
            { }

        /**
         * Returns a pointer to the current element.
         * @return the element pointer, possibly null
         */
        T *ptr() const
            { return (T *)link->data; }

	WvIterStuff(T);
	
        /**
         * Unlinks the current element from the list and automatically
         * increments the iterator to point to the next element as if
         * next() had been called.
         */
        void unlink()
        {
	    if (prev) ((WvList *)list)->unlink_after(prev);
	    link = prev->next;
        }
	
        /**
         * Unlinks the current element from the list but unlike unlink()
         * automatically returns the iterator to the previous link in
         * the list such that next() must be called to obtain the
         * next element.
         * <p>
         * This version allows for writing neater loop structures since
         * an element can be unlinked in mid-traversal while still allowing
         * the iterator to be incremented at the top of the loop as usual.
         * </p><p>
         * Calling xunlink() twice in a row is currently unsupported.
         * </p>
         */
	void xunlink()
	{
	    if (prev) ((WvList *)list)->unlink_after(prev);
	    link = prev;
	}
    };
    
    /**
     * The sorted iterator type for linked lists.
     */
    typedef class WvSorter<T, WvListBase, WvListBase::IterBase> Sorter;
};


#define DeclareWvList4(_type_,_newname_,_fullname_,_extra_) \
    class _fullname_ : public WvList<_type_> 		\
    { 							\
    public: 						\
        _newname_() { setup(); }			\
        						\
        ~_newname_() { shutdown(); }			\
	_extra_ 					\
    };

#define DeclareWvList3(_type_,_newname_,_extra_) \
    DeclareWvList4(_type_,_newname_,_newname_,_extra_)

#define DeclareWvList2(_type_,_extra_)  		\
		DeclareWvList3(_type_,_type_##List,_extra_ )

#define DeclareWvList(_type_) DeclareWvList2(_type_,;)


#endif // __WVLINKLIST_H
