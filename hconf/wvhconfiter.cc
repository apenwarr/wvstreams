/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Several kinds of WvHConf iterators.
 */
#include "wvhconfiter.h"


WvLink *WvHConf::RecursiveIter::_next()
{ 
    if (!subiter && i.ptr() && i->children)
    {
	subiter = new RecursiveIter(*i);
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


static int find_wildcard_depth(const WvHConfKey &k)
{
    int depth = 0;
    WvHConfKey::Iter ki(k);
    for (ki.rewind(); ki.next(); )
    {
	if (*ki == "*") // wildcard element found!
	    break;
	depth++;
    }
    return depth;
}
    

// I hate constructors.
WvHConf::XIter::XIter(WvHConf &_top, const WvHConfKey &_key) 
	: skiplevel(find_wildcard_depth(_key)),
          top(_top.find(_key.header(skiplevel))),
          key(_key.skip(skiplevel)),
          _toplink(top, false), toplink(top ? &_toplink : NULL),
          i((top && top->children) ? *top->children : null_wvhconfdict)
{
    subiter = NULL; 
}


WvLink *WvHConf::XIter::_next()
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
	    subiter = new XIter(*i, key.skip(1));
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
