/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * Several kinds of UniConf iterators.
 */
#include "uniconfiter.h"

/***** UniConf::Iter *****/

UniConf::Iter::Iter(UniConf &_root) :
    root(_root), it(_root)
{
}


void UniConf::Iter::rewind()
{
    root.check_children();
    it.rewind();
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(UniConf &h) :
    top(h)
{
}


void UniConf::RecursiveIter::rewind()
{
    itlist.zap();
    itlist.append(& top, false);
    top.rewind();
}


bool UniConf::RecursiveIter::next()
{
    IterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::Iter &it = itlistit();
        if (it.next())
        {
            current = it.ptr();
            if (current->check_children())
            {
                UniConf::Iter *subit = new UniConf::Iter(*current);
                subit->rewind();
                itlist.prepend(subit, true);
            }
            return true;
        }
        itlistit.xunlink();
    }
    return false;
}



#if 0
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
#endif
