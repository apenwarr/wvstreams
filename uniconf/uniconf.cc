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

// basic constructor, generally used for toplevel config file
UniConf::UniConf() :
    UniConfTree(NULL, UniConfKey::EMPTY, WvString::null)
{
    init();
}


UniConf::UniConf(UniConf *_parent, const UniConfKey &_name) :
    UniConfTree(_parent, _name, WvString::null)
{
    init();
}


void UniConf::init()
{
    defaults = NULL;
    generator = NULL;
    
    dirty  = child_dirty  = false;
    notify = child_notify = false;
    waiting = child_waiting = false;
    obsolete = child_obsolete = false;
}


UniConf::~UniConf()
{
    if (generator)
	delete generator;
}


// find the topmost UniConf object in this tree.  Using this too often might
// upset the clever transparency of hierarchical nesting, but sometimes it's
// a good idea (particularly if you need to use full_key()).
UniConf *UniConf::top()
{
    UniConf *h = this;
    while (h->parent())
	h = h->parent();
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
	k.prepend(h->key());
	h = h->parent();
    }
    return k;
}


// Find the controlling generator instance
UniConf *UniConf::gen_top()
{
    UniConf *h = this;
    while (h->parent() && ! h->hasgen())
	h = h->parent();
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

bool UniConf::check_children()
{
    if (hasgen())
    {
        generator->enumerate_subtrees(this, false);
    }
    else
    {
        // quick hack for now
        UniConf *top = gen_top();
        if (top != this)
            top->generator->enumerate_subtrees(top, true);
    }
    return haschildren();
}

void UniConf::remove(const UniConfKey &key)
{
    UniConf *toremove = find(key);
    
    if (!toremove)
    {
        wvcon->print("Could not remove %s.\n", key);
        return;
    }

    toremove->setvalue(WvString::null);
    toremove->dirty = true;
    //toremove->deleted = true;
    UniConf::RecursiveIter i(*toremove);

    for (i.rewind(); i.next();)
    {
        i->setvalue(WvString::null);
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
UniConf *UniConf::findormake(const UniConfKey &key)
{
    UniConf *tree = static_cast<UniConf*>(UniConfTree::find(key));
    if (tree)
        return tree;

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
    
    if (parent())
    {
	// go up a level and try again
	UniConf *def = parent()->find_default(UniConfKey(key(), k));
	if (def)
	    return def;
	
	// try with a wildcard instead
	def = parent()->find_default(UniConfKey(UniConfKey::ANY, k));
	if (def)
	    return def;
    }
    
    return NULL;
}


WvString UniConf::get(const UniConfKey &key)
{
    UniConf *tree = find(key);
    if (tree && tree->value())
        return tree->value();

    UniConf *def = find_default();
    if (def)
        return def->get(key);
    return WvString::null;
}


void UniConf::set(const UniConfKey &key, WvStringParm _value)
{
    if (! key.isempty())
    {
        findormake(key)->set(UniConfKey::EMPTY, _value);
        return;
    }
    if (value() == _value)
	return; // nothing to change - no notifications needed
    setvalue(_value);
    
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
    
    h = parent();
    while (h && (!h->child_dirty || !h->child_notify))
    {
	h->child_dirty = h->child_notify = true;
	h = h->parent();
    }
}


void UniConf::load()
{
    if (generator)
	generator->load();
    else if (haschildren())
    {
	UniConf::Iter it(*this);
	for (it.rewind(); it.next(); )
	    it->load();
    }
}


void UniConf::save()
{
    if (!dirty && !child_dirty)
	return; // done!
    
    if (generator)
	generator->save();
    else if (haschildren() && child_dirty)
    {
        UniConf::Iter it(*this);
	for (it.rewind(); it.next(); )
	    it->save();
    }
}


void UniConf::_dump(WvStream &s, bool everything, WvStringTable &keytable)
{
    WvString key(full_key());
    
    if (everything || !!value() || keytable[key])
    {
	s.print("  %s%s%s%s%s%s %s = %s\n",
	        child_dirty, dirty,
		child_notify, notify,
		child_obsolete, obsolete,
		key, value());
    }

    // this key better not exist yet!
    assert(!keytable[key]);
    
    keytable.add(new WvString(key), true);
    
    if (haschildren())
    {
	UniConf::Iter it(*this);
	for (it.rewind(); it.next(); )
	    it->_dump(s, everything, keytable);
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
