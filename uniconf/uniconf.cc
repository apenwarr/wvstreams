/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 * 
 * See uniconf.h.
 */
#include "uniconf.h"
#include "wvstream.h"
#include "wvstringtable.h"
#include "uniconfiter.h"
#include <assert.h>

UniConfDict null_wvhconfdict(1);


// basic constructor, generally used for toplevel config file
UniConf::UniConf()
    : name("")
{
    parent = NULL;
    init();
}


UniConf::UniConf(UniConf *_parent, const UniConfKey &_name)
    : name(_name)
{
    parent = _parent;
    init();
}


void UniConf::init()
{
    children = NULL;
    defaults = NULL;
    generator = NULL;
    
    dirty  = child_dirty  = false;
    notify = child_notify = false;
    waiting = child_waiting = false;
    obsolete = child_obsolete = false;
}


UniConf::~UniConf()
{
    if (children)
	delete children;
    if (generator)
	delete generator;
}


// find the topmost UniConf object in this tree.  Using this too often might
// upset the clever transparency of hierarchical nesting, but sometimes it's
// a good idea (particularly if you need to use full_key()).
UniConf *UniConf::top()
{
    UniConf *h = this;
    while (h->parent)
	h = h->parent;
    return h;
}


// this method of returning the object is pretty inefficient - lots of extra
// copying stuff around.
UniConfKey UniConf::full_key(UniConf *top) const
{
    UniConfKey k;
    const UniConf *h = this;
    
    while (h && h != top)
    {
	k.prepend(h->name);
	h = h->parent;
    }
    return k;
}


// Find the controlling generator instance
UniConf *UniConf::gen_top()
{
    UniConf *h = this;
    while (h->parent && ! h->hasgen())
	h = h->parent;
    return h; // we reached the top of the tree without finding a generator.
}


// Figure out the full key for this location, but based from the gen_top()
// instead of the top().
// 
// like with gen_key, this method of returning the object is pretty
// inefficient - lots of extra copying stuff around.
UniConfKey UniConf::gen_full_key()
{
    return full_key(gen_top());
}

bool UniConf::check_children(bool recursive)
{
    if (checkgen())
    {
        this->generator->enumerate_subtrees(this, recursive);
    }
    return (children != NULL);
}

// find a key in the subtree.  If it doesn't already exist, return NULL.
UniConf *UniConf::find(const UniConfKey &key)
{
    if (key.isempty())
	return this;
    if (!children)
	return NULL;
    
    UniConf *h = (*children)[key.first(1)];
    if (!h)
	return NULL;
    else
	return h->find(key.removefirst(1));
}

void UniConf::remove(const UniConfKey &key)
{
    UniConf *toremove = find(key);
    
    if (!toremove)
    {
        wvcon->print("Could not remove %s.\n", key);
        return;
    }

    toremove->set(WvString());
    toremove->dirty = true;
    //toremove->deleted = true;
    UniConf::RecursiveIter i(*toremove);

    for (i.rewind(); i._next();)
    {
        i->set(WvString());
        i->dirty = true;
//        i->deleted = true;
//        i->child_deleted = true;
    }

/*    for (UniConf *par = toremove->parent; par != NULL; par = par->parent)
    {
        par->child_deleted = true;
    }*/
}

// find a key in the subtree.  If it doesn't already exist, create it.
UniConf *UniConf::find_make(const UniConfKey &key)
{
    if (key.isempty())
	return this;


    if (children)
    {
	UniConf *h = (*children)[key.first(1)];
	if (h)
	    return h->find_make(key.removefirst(1));
    }
	
    // we need to actually create the key
    UniConf *htop = gen_top();
    if (htop->generator)
    {
        UniConf *toreturn = htop->generator->make_tree(this, key);
       
        // avoid an infinite loop... but at the same time check to see
        // that we can get our info.
        while(toreturn->waiting || toreturn->obsolete)
        {
            htop->generator->update(toreturn);
        }
        return toreturn;
    }
    else
	return UniConfGen().make_tree(this, key); // generate an empty tree
}

void UniConf::update()
{
    UniConf *htop = gen_top();
    UniConf *me = this;
    while (obsolete || waiting && !dirty)
    {
        htop->generator->update(me);
    }
}

// find a key that contains the default value for this key, regardless of
// whether this key _needs_ a default value or not.  (If !!value, we no longer
// need a default, but this function still can return non-NULL.)
// 
// If there's no available default value for this key, return NULL.
// 
UniConf *UniConf::find_default(const UniConfKey &k) const
{
    //wvcon->print("find_default for '%s'\n", full_key());
	
    if (defaults)
    {
	//wvcon->print("  find key '%s' in '%s'\n", k, defaults->full_key());
	UniConf *def = defaults->find(k);
	if (def)
	    return def;
    }
    
    if (parent)
    {
	// go up a level and try again
	UniConf *def = parent->find_default(UniConfKey(name, k));
	if (def)
	    return def;
	
	// try with a wildcard instead
	def = parent->find_default(UniConfKey(UniConfKey::ANY, k));
	if (def)
	    return def;
    }
    
    return NULL;
}


void UniConf::set_without_notify(WvStringParm s)
{
    value = s;
}


void UniConf::set(WvStringParm s)
{
    if (s == value)
	return; // nothing to change - no notifications needed
    
    set_without_notify(s);
    
    if (dirty && notify)
	return; // nothing more needed
    
    mark_notify();
}


// set the dirty and notify flags on this object, and inform all parent
// objects that their child is dirty.
void UniConf::mark_notify()
{
    UniConf *h;
    
    dirty = notify = true;
    
    h = parent;
    while (h && (!h->child_dirty || !h->child_notify))
    {
	h->child_dirty = h->child_notify = true;
	h = h->parent;
    }
}


const WvString &UniConf::printable() const
{
    UniConf *def;
   
    if (!!value)
	return value;
    
    def = find_default();
    if (def)
	return def->printable();
    
    // no default found: return the value (basically NULL) anyway
    return value;
}


void UniConf::load()
{
    if (generator)
	generator->load();
    else if (children)
    {
	UniConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->load();
    }
}


void UniConf::save()
{
    if (!dirty && !child_dirty)
	return; // done!
    
    if (generator)
	generator->save();
    else if (children && child_dirty)
    {
	UniConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->save();
    }
}


void UniConf::_dump(WvStream &s, bool everything, WvStringTable &keytable)
{
    WvString key(full_key());
    
    if (everything || !!value || keytable[key])
    {
	s.print("  %s%s%s%s%s%s %s = %s\n",
	        child_dirty, dirty,
		child_notify, notify,
		child_obsolete, obsolete,
		key, value);
    }

    // this key better not exist yet!
    assert(!keytable[key]);
    
    keytable.add(new WvString(key), true);
    
    if (children)
    {
	UniConfDict::Iter i(*children);
	for (i.rewind(); i.next(); )
	    i->_dump(s, everything, keytable);
    }
}


void UniConf::dump(WvStream &s, bool everything)
{
    WvStringTable keytable(100);
    _dump(s, everything, keytable);
}

void UniConf::mount(UniConfGen *gen)
{
    this->unmount();

    this->generator = gen;
    this->generator->load();
}

void UniConf::unmount()
{
    if (generator)
        delete generator;
    generator = NULL;
}
