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
	    pre_get(child);
	}
	
	parent = child;
    }
 
    update(parent);
    return parent;
}

void UniConfGen::enumerate_subtrees(UniConf *conf, bool recursive)
{
    // do nothing by default.
}

void UniConfGen::pre_get(UniConf *&h)
{
    // do nothing by default.
}

void UniConfGen::update_all()
{
    // do nothing 
}

void UniConfGen::update(UniConf *&h)
{
    // do nothing by default.
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
