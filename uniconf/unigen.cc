/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen knows how to generate new UniConf objects in its tree.
 * 
 * See uniconf.h.
 */
#include "uniconf.h"


UniConfGen::~UniConfGen()
{
    // nothing special
}


UniConf *UniConfGen::make_tree(UniConf *parent, const UniConfKey &key)
{
    UniConfKey::Iter i(key);
    for (i.rewind(); i.next(); )
    {
	if (!parent->children)
	    parent->children = new UniConfDict(10);
	
	UniConf *child = (*parent->children)[*key.first()];
	if (!child)
	{
	    child = new UniConf(parent, *i);
	    parent->children->add(child, true);
	    update(child);
	}
	
	parent = child;
    }
    
    return parent;
}


void UniConfGen::update(UniConf *h)
{
    h->dirty = false;
}


void UniConfGen::load()
{
    // do nothing by default
}


void UniConfGen::save()
{
    // do nothing by default
}

