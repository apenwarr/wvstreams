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
	
	UniConf *child = (*parent->children)[*i];
	if (!child)
	{
	    child = new UniConf(parent, *i);
	    parent->children->add(child, true);
            child->waiting = true;
	    update(child);
	}
	
	parent = child;
    }
    
    return parent;
}

void UniConfGen::enumerate_subtrees(const UniConfKey &key)
{
    // do nothing by default.
}

void UniConfGen::update(UniConf *&h)
{
    h->dirty = false;
    h->waiting = false;
    h->obsolete = false;
}


void UniConfGen::load()
{
    // do nothing by default
}


void UniConfGen::save()
{
    // do nothing by default
}
