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
#include "uniconfnull.h"
#include "wvstream.h"
#include "wvstringtable.h"
#include "uniconfiter.h"
#include <assert.h>

/***** UniConf *****/

// basic constructor, generally used for toplevel config file
UniConf::UniConf() :
    UniConfNotifyTree(NULL, UniConfKey::EMPTY, WvString::null)
{
    init();
}


UniConf::UniConf(UniConf *_parent, const UniConfKey &_name) :
    UniConfNotifyTree(_parent, _name, WvString::null)
{
    init();
}


void UniConf::init()
{
    defaults = NULL;
    generator = NULL;
}


UniConf::~UniConf()
{
    if (generator)
	delete generator;
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
    return fullkey(gen_top());
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
        if (top != this && top->hasgen())
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
    UniConf *tree = static_cast<UniConf*>(UniConfNotifyTree::find(key));
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
    {
        // generate an empty tree
	return UniConfNullGen().make_tree(this, key);
    }
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

    marknotify();
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


void UniConf::dump(WvStream &stream, bool everything)
{
    UniConf::RecursiveIter it(*this);
    for (it.rewind(); it.next(); )
    {
        if (everything || !! it->value())
            stream.print("%s = %s\n", it->fullkey(), it->value());
    }
}

bool UniConf::mount(const UniConfLocation &location)
{
    unmount();
    return mount(UniConfGenFactoryRegistry::instance()->
        newgen(location, this));
}


bool UniConf::mount(UniConfGen *gen)
{
    unmount();
    if (gen)
    {
        generator = gen;
        generator->load();
        return true;
    }
    return false;
}

void UniConf::unmount()
{
    delete generator;
    generator = NULL;
}
