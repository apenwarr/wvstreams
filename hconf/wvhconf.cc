/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 * 
 * See wvhconf.h.
 */
#include "wvhconf.h"
#include "wvstream.h"


// basic constructor, generally used for toplevel config file
WvHConf::WvHConf()
    : name("")
{
    parent = NULL;
    init();
}


WvHConf::WvHConf(WvHConf *_parent, WvStringParm _name)
    : name(_name)
{
    parent = _parent;
    init();
}


void WvHConf::init()
{
    children = NULL;
    defaults = NULL;
    generator = NULL;
    
    dirty  = child_dirty  = false;
    notify = child_notify = false;
    
    obsolete = child_obsolete = false;
}


WvHConf::~WvHConf()
{
    if (children)
	delete children;
    if (generator)
	delete generator;
}


// find the topmost WvHConf object in this tree.  Using this too often might
// upset the clever transparency of hierarchical nesting, but sometimes it's
// a good idea (particularly if you need to use full_key()).
WvHConf *WvHConf::top()
{
    WvHConf *h = this;
    while (h->parent)
	h = h->parent;
    return h;
}


// this method of returning the object is pretty inefficient - lots of extra
// copying stuff around.
WvHConfKey WvHConf::full_key() const
{
    WvHConfKey k;
    const WvHConf *h = this;
    
    do
    {
	k.prepend(new WvString(h->name), true);
	h = h->parent;
    } while (h);
    
    return k;
}


// find the topmost WvHConf object in the tree that's still owned by the
// same WvHConfGen object.
WvHConf *WvHConf::gen_top()
{
    WvHConf *h = this;
    while (h->parent && !h->generator)
	h = h->parent;
    
    return h; // we reached the top of the tree without finding a generator.
}


// Figure out the full key for this location, but based from the gen_top()
// instead of the top().
// 
// like with gen_key, this method of returning the object is pretty
// inefficient - lots of extra copying stuff around.
WvHConfKey WvHConf::gen_full_key() const
{
    WvHConfKey k;
    const WvHConf *h = this;
    
    while (h && !h->generator)
    {
	k.prepend(new WvString(h->name), true);
	h = h->parent;
    };
    
    return k;
}


// find a key in the subtree.  If it doesn't already exist, return NULL.
WvHConf *WvHConf::find(const WvHConfKey &key)
{
    if (key.isempty())
	return this;
    if (!children)
	return NULL;
    
    WvHConf *h = (*children)[*key.first()];
    if (!h)
	return NULL;
    else
	return h->find(key.skip(1));
}


// find a key in the subtree.  If it doesn't already exist, create it.
WvHConf *WvHConf::find_make(const WvHConfKey &key)
{
    if (key.isempty())
	return this;
    if (children)
    {
	WvHConf *h = (*children)[*key.first()];
	if (h)
	    return h->find_make(key.skip(1));
    }
	
    // we need to actually create the key
    WvHConf *htop = gen_top();
    if (htop->generator)
	return htop->generator->make_tree(this, key);
    else
	return WvHConfGen().make_tree(this, key); // generate an empty tree
}


// find a key that contains the default value for this key, regardless of
// whether this key _needs_ a default value or not.  (If !!value, we no longer
// need a default, but this function still can return non-NULL.)
// 
// If there's no available default value for this key, return NULL.
// 
WvHConf *WvHConf::find_default(WvHConfKey *_k) const
{
    WvHConfKey tmp_key;
    WvHConfKey &k = (_k ? *_k : tmp_key);
    WvHConf *def;
    
    //wvcon->print("find_default for '%s'\n", full_key());
	
    if (defaults)
    {
	//wvcon->print("  find key '%s' in '%s'\n", k, defaults->full_key());
	def = defaults->find(k);
	if (def)
	    return def;
    }
    
    if (parent)
    {
	// go up a level and try again
	WvString s1(name);
	k.prepend(&s1, false);
	def = parent->find_default(&k);
	k.unlink_first();
	if (def)
	    return def;
	
	// try with a wildcard instead
	WvString s2("*");
	k.prepend(&s2, false);
	def = parent->find_default(&k);
	k.unlink_first();
	if (def)
	    return def;
    }
    
    return NULL;
}


void WvHConf::set_without_notify(WvStringParm s)
{
    value = s;
    value.unique();
}


void WvHConf::set(WvStringParm s)
{
    WvHConf *h;
    
    if (s == value)
	return; // nothing to change - no notifications needed
    
    set_without_notify(s);
    
    if (dirty && notify)
	return; // nothing more needed
    
    dirty = notify = true;
    
    // also notify all parents that a child has changed.  We can stop this
    // if we reach a parent that already has child_notify AND child_dirty
    // set to true.
    h = parent;
    while (h && (!h->child_dirty || !h->child_notify))
    {
	h->child_dirty = h->child_notify = true;
	h = h->parent;
    }
}


void WvHConf::do_notify()
{
    WvHConf *h;
    
    dirty = notify = true;
    
    h = parent;
    while (h && (!h->child_dirty || !h->child_notify))
    {
	h->child_dirty = h->child_notify = true;
	h = h->parent;
    }
}


const WvString &WvHConf::printable() const
{
    WvHConf *def;
    
    if (!!value)
	return value;
    
    def = find_default();
    if (def)
	return def->printable();
    
    // no default found: return the value (basically NULL) anyway
    return value;
}


void WvHConf::load()
{
    if (generator)
	generator->load();
    else if (children)
    {
	WvHConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->load();
    }
}


void WvHConf::save()
{
    if (!dirty && !child_dirty)
	return; // done!
    
    if (generator)
	generator->save();
    else if (children && child_dirty)
    {
	WvHConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->save();
    }
}


void WvHConf::dump(WvStream &s)
{
    if (!!value)
    {
	s.print("  %s%s%s%s%s%s %s = %s\n",
	        child_dirty, dirty,
		child_notify, notify,
		child_obsolete, obsolete,
		full_key(), value);
    }

    if (children)
    {
	WvHConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->dump(s);
    }
}


