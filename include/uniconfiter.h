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
#include "wvlinklist.h"


/**
 * This iterator walks through all the immediate children of a
 * UniConf node.
 */
class UniConf::Iter
{
    UniConf &root;
    UniConfNotifyTree::Iter it;

public:
    Iter(UniConf &root);

    void rewind();

    inline bool next()
    {
        return it.next();
    }
    inline UniConf *ptr() const
    {
        return static_cast<UniConf*>(it.ptr());
    }
    WvIterStuff(UniConf);
};



/**
 * This iterator performs pre-order traversal of all of the
 * children in a UniConf tree.
 */
class UniConf::RecursiveIter
{
    DeclareWvList3(UniConf::Iter, IterList, )
    UniConf::Iter top;
    UniConf *current;
    IterList itlist;

public:
    RecursiveIter(UniConf &h);

    void rewind();
    bool next();

    inline UniConf *ptr() const
    {
        return current;
    }
    WvIterStuff(UniConf);
};


#if 0
/**
 * FIXME: WHAT DOES THIS DO?
 */
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
    
    WvLink *next()
    {
	WvLink *l;
	while ((l = _next()) != NULL && ptr() == NULL);
	return l;
    }
    
    UniConf *ptr() const
    {
        return key.isempty() ?
            top : (subiter ? subiter->ptr() : NULL);
    }
    
    WvIterStuff(UniConf);
};
#endif


#if 0
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
#endif

#endif // __UNICONFITER_H
