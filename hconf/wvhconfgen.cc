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
    if (key.isempty())
	return parent;
    
    WvHConf *h = parent->children ? (*parent->children)[*key.first()] : NULL;

    if (!h)
	h = make_obj(parent, *key.first());
    if (!h)
	return NULL;
    
    return make_tree(h, key.skip(1));
}


WvHConf *WvHConfGen::make_obj(WvHConf *parent, WvStringParm name)
{
    WvHConf *child = new WvHConf(parent, name);
    if (!parent->children)
	parent->children = new WvHConfDict(10);
    parent->children->add(child, true);
    update(child);
    return child;
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

