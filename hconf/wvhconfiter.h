/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Several kinds of WvHConf iterators.
 */
#ifndef __WVHCONFITER_H
#define __WVHCONFITER_H

#include "wvhconf.h"


// this iterator walks through all the immediate children of a
// WvHConf node.
class WvHConf::Iter : public WvHConfDict::Iter
{
public:
    Iter(WvHConf &h)
	: WvHConfDict::Iter(h.children ? *h.children : null_wvhconfdict)
	{ }
    Iter(WvHConfDict &children)
	: WvHConfDict::Iter(children)
	{ }
    
    // we want to skip empty-valued elements in the list, even if
    // they exist.
    WvLink *next()
    {
	WvLink *l;
	while ((l = WvHConfDict::Iter::next()) != NULL 
	       && !*ptr() && !ptr()->children)
	    ;
	return l;
    }
};


// this iterator recursively walks through _all_ children, direct and indirect,
// of this node.
class WvHConf::RecursiveIter
{
public:
    WvHConfDict::Iter i;
    RecursiveIter *subiter;
    
    RecursiveIter(WvHConf &h)
	: i(h.children ? *h.children : null_wvhconfdict)
	{ subiter = NULL; }
    RecursiveIter(WvHConfDict &children)
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
    
    // like _next(), but skip elements with empty values.
    WvLink *next()
    { 
	WvLink *l;
	while ((l = _next()) != NULL && !*ptr())
	    ;
	return l;
    }
    
    WvHConf *ptr() const
        { return subiter ? subiter->ptr() : i.ptr(); }
    
    WvIterStuff(WvHConf);
};


class WvHConf::XIter
{
public:
    int skiplevel;
    WvHConf *top;
    WvHConfKey key;
    WvLink _toplink, *toplink;
    WvHConfDict::Iter i;
    XIter *subiter;
    int going;
    
    XIter(WvHConf &_top, const WvHConfKey &_key);
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
    
    WvHConf *ptr() const
        { return key.isempty() ? top : (subiter ? subiter->ptr() : NULL); }
    
    WvIterStuff(WvHConf);
};


// WvHConf::Sorter is like WvHConf::Iter, but allows you to sort the list.
typedef WvSorter<WvHConf, WvHConfDict, WvHConf::Iter>
    _WvHConfSorter;
class WvHConf::Sorter : public _WvHConfSorter
{
public:
    Sorter(WvHConf &h, RealCompareFunc *cmp)
	: _WvHConfSorter(h.children ? *h.children : null_wvhconfdict,
			 cmp)
	{ }
};


// WvHConf::RecursiveSorter is the recursive version of WvHConf::Sorter.
typedef WvSorter<WvHConf, WvHConfDict, WvHConf::RecursiveIter> 
    _WvHConfRecursiveSorter;
class WvHConf::RecursiveSorter : public _WvHConfRecursiveSorter
{
public:
    RecursiveSorter(WvHConf &h, RealCompareFunc *cmp)
	: _WvHConfRecursiveSorter(h.children ? *h.children : null_wvhconfdict,
				  cmp)
	{ }
};


#endif // __WVHCONFITER_H
