/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A WvHConfGen knows how to generate new WvHConf objects in its tree.
 * 
 * See wvhconf.h.
 */
#include "wvhconf.h"


WvHConfGen::~WvHConfGen()
{
    // nothing special
}


WvHConf *WvHConfGen::make_tree(WvHConf *parent, const WvHConfKey &key)
{
    WvHConfKey::Iter i(key);
    for (i.rewind(); i.next(); )
    {
	if (!parent->children)
	    parent->children = new WvHConfDict(10);
	
	WvHConf *child = (*parent->children)[*key.first()];
	if (!child)
	{
	    child = new WvHConf(parent, *i);
	    parent->children->add(child, true);
	    update(child);
	}
	
	parent = child;
    }
    
    return parent;
}


void WvHConfGen::update(WvHConf *h)
{
    h->dirty = false;
}


void WvHConfGen::load()
{
    // do nothing by default
}


void WvHConfGen::save()
{
    // do nothing by default
}

