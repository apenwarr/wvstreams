/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Several kinds of UniConf iterators.
 */
#ifndef __UNICONFITER_H
#define __UNICONFITER_H

#include "uniconf.h"


// this iterator walks through all the immediate children of a
// UniConf node.
class UniConf::Iter : public UniConfDict::Iter
{
public:
    Iter(UniConf &h)
	: UniConfDict::Iter(h.check_children() ? *h.children : null_wvhconfdict)
	{ }
    Iter(UniConfDict &children)
	: UniConfDict::Iter(children)
	{ }
    
    // we want to skip empty-valued elements in the list, even if
    // they exist.
    WvLink *next()
    {
	WvLink *l;
	while ((l = UniConfDict::Iter::next()) != NULL 
	       && !*ptr() && !ptr()->children)
	    ;
	return l;
    }

};


// this iterator recursively walks through _all_ children, direct and indirect,
// of this node.
class UniConf::RecursiveIter
{
public:
    UniConfDict::Iter i;
    RecursiveIter *subiter;
    bool recursed_children; // FIXME:  Quick hack to speed up recursive iterators for huge lists.
    
    RecursiveIter(UniConf &h)
	: i(h.check_children() ? *h.children : null_wvhconfdict), recursed_children(false)
	{ subiter = NULL; }
    RecursiveIter(UniConfDict &children)
	: i(children)
	{ subiter = NULL; }
    ~RecursiveIter()
        { unsub(); }
    
    void unsub()
        { if (subiter) delete subiter; subiter = NULL; }
    
    void rewind()
        { unsub(); i.rewind(); }
    
    WvLink *cur()
        { return subiter ? subiter->cur() : i.cur(); }
    
    // return the next element, either from subiter or, if subiter is done,
    // the next immediate child of our own.
    WvLink *_next();
    
    // like _next(), but skip elements with empty values that are not mountpoints for generators.
    WvLink *next()
    { 
	WvLink *l;
	while ((l = _next()) != NULL && !*ptr() && !ptr()->hasgen());
	    ;
	return l;
    }
    
    UniConf *ptr() const
        { return subiter ? subiter->ptr() : i.ptr(); }
    
    WvIterStuff(UniConf);
};


class UniConf::XIter
{
public:
    int skiplevel;
    UniConf *top;
    UniConfKey key;
    WvLink _toplink, *toplink;
    UniConfDict::Iter i;
    XIter *subiter;
    int going;
    
    XIter(UniConf &_top, const UniConfKey &_key);
    ~XIter()
        { unsub(); }
    
    void unsub()
        { if (subiter) delete subiter; subiter = NULL; }
    
    void rewind()
        { unsub(); i.rewind(); going = 0; }
    
    WvLink *cur()
        { return subiter ? subiter->cur() : (going==1 ? toplink : NULL); }
    
    WvLink *_next();
    
    // we want to skip empty-valued elements in the list, even if
    // they exist.
    WvLink *next()
    {
	WvLink *l;
	while ((l = _next()) != NULL && !*ptr())
	    ;
	return l;
    }
    
    UniConf *ptr() const
        { return key.isempty() ? top : (subiter ? subiter->ptr() : NULL); }
    
    WvIterStuff(UniConf);
};


// UniConf::Sorter is like UniConf::Iter, but allows you to sort the list.
typedef WvSorter<UniConf, UniConfDict, UniConf::Iter>
    _UniConfSorter;
class UniConf::Sorter : public _UniConfSorter
{
public:
    Sorter(UniConf &h, RealCompareFunc *cmp)
	: _UniConfSorter(h.check_children() ? *h.children : null_wvhconfdict,
			 cmp)
	{ }
};


// UniConf::RecursiveSorter is the recursive version of UniConf::Sorter.
typedef WvSorter<UniConf, UniConfDict, UniConf::RecursiveIter> 
    _UniConfRecursiveSorter;
class UniConf::RecursiveSorter : public _UniConfRecursiveSorter
{
public:
    RecursiveSorter(UniConf &h, RealCompareFunc *cmp)
	: _UniConfRecursiveSorter(h.check_children() ? *h.children : null_wvhconfdict,
				  cmp)
	{ }
};


#endif // __UNICONFITER_H
