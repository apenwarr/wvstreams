/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Several kinds of UniConf iterators.
 */
#include "uniconfiter.h"
#include "wvstream.h"

WvLink *UniConf::RecursiveIter::_next()
{ 
    if (i.ptr() && i->generator && !recursed_children)
    {
//        wvcon->print("Recursively checking children for %s.\n", (int)i.ptr());
        i->check_children(true);
        recursed_children = true;
    }

    if (!subiter && i.ptr() && i->children)
    {
	subiter = new RecursiveIter(*i->children);
	subiter->rewind();
    }
    
    if (subiter)
    {
	WvLink *l = subiter->next();
	if (l) return l;
	unsub();
    }
    
    return i.next();
}


static int find_wildcard_depth(const UniConfKey &k)
{
    int depth = 0;
    int segments = k.numsegments();
    while (depth < segments)
    {
        if (k.segment(depth) == UniConfKey::ANY)
            break;
	depth++;
    }
    return depth;
}
    

// I hate constructors.
UniConf::XIter::XIter(UniConf &_top, const UniConfKey &_key) :
    skiplevel(find_wildcard_depth(_key)),
    top(_top.find(_key.first(skiplevel))),
    key(_key.removefirst(skiplevel)),
    _toplink(top, false),
    toplink(top ? &_toplink : NULL),
    i((top && top->check_children()) ?
        *top->children : null_wvhconfdict)
{
    subiter = NULL; 
}


WvLink *UniConf::XIter::_next()
{
    if (key.isempty()) // we're the innermost object
    {
	if (++going == 1)
	    return toplink;
	else
	    return NULL;
    }
    
    do
    {
	if (!subiter && i.ptr())
	{
	    subiter = new XIter(*i, key.removefirst(1));
	    subiter->rewind();
	}
	
	if (subiter)
	{
	    WvLink *l = subiter->next();
	    if (l) return l;
	    unsub();
	}
    } while (i.next());
    
    return NULL;
}
