/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "wvlog.h"
#include "wvhashtable.h"
#include "wvstringlist.h"

class WvHConf;
class WvHConfDict;

class WvHConfKey : public WvStringList
{
public:
    WvHConfKey();
    WvHConfKey(const char *key);
    WvHConfKey(const WvString &key);
    WvHConfKey(const WvString &section, const WvString &entry);
    WvHConfKey(const WvHConfKey &key, int offset = 0);
    
    WvString printable() const;
    operator WvString () const { return printable(); }
    
    WvHConfKey skip(int offset) const
        { return WvHConfKey(*this, offset); }
};


class WvHConf
{
public:
    WvHConf *parent;       // the 'parent' of this subtree
    WvString name;         // any node contains 'name = value' data
    WvString value;
    WvHConf *defaults;     // a tree possibly containing default values
    WvHConfDict *children; // list of all child nodes of this node (subkeys)
    
    WvHConf();
    WvHConf(WvHConf *_parent, const WvString &_name);
    ~WvHConf();
    
    WvHConf *top();
    WvHConfKey full_key() const;
    
    WvHConf *find(const WvHConfKey &key);
    WvHConf *find_make(const WvHConfKey &key);
    WvHConf &operator[](const WvHConfKey &key) { return *find_make(key); }
    
    WvHConf *find_default(WvHConfKey *_k = NULL) const;
    
    WvString &get(const WvHConfKey &key)
        { return find_make(key)->value; }
    
    void set(const WvHConfKey &key, const WvString &value)
        { find_make(key)->value = value; }
    
    WvString &operator= (const WvString &s) { return value = s; }
    
    const WvString& printable() const;
    operator const WvString& () { return printable(); }
};

DeclareWvDict(WvHConf, WvString, name);



// null constructor: let people fill it by hand later
WvHConfKey::WvHConfKey()
{
    // leave it empty
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
WvHConfKey::WvHConfKey(const WvString &key)
{
    split(key, "/");
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
WvHConfKey::WvHConfKey(const char *key)
{
    split(key, "/");
}


// old-style 2-level key: /section/entry.
WvHConfKey::WvHConfKey(const WvString &section, const WvString &entry)
{
    append(new WvString(section), true);
    append(new WvString(entry), true);
}


// copy an old key to this key, stripping the leading components.
// This isn't a very efficient copy operation, but maybe that's okay...
WvHConfKey::WvHConfKey(const WvHConfKey &key, int offset)
{
    int count = 0;
    Iter i(key);
    
    for (count = 0, i.rewind(); count < offset && i.next(); count++)
	; // do nothing; just skipping stuff.
    if (!i.cur())
	return;
    while (i.next())
	append(new WvString(*i), true);
}


WvString WvHConfKey::printable() const
{
    return join("/");
}


// basic constructor, generally used for toplevel config file
WvHConf::WvHConf()
    : name("")
{
    parent = NULL;
    defaults = NULL;
    children = NULL;
}


WvHConf::WvHConf(WvHConf *_parent, const WvString &_name)
    : name(_name)
{
    parent = _parent;
    defaults = NULL;
    children = NULL;
}


WvHConf::~WvHConf()
{
    if (children)
	delete children;
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
    if (!children)
	children = new WvHConfDict(10);
    
    WvHConf *h = (*children)[*key.first()];
    
    if (h)
	return h->find_make(key.skip(1));
    else
    {
	// create a subkey, then call it recursively.
	WvHConf *child = new WvHConf(this, *key.first());
	children->add(child, true);
	return child->find_make(key.skip(1));
    }
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


void dumpcfg(WvHConf &cfg, WvStream &s)
{
    if (!!cfg.value)
	s.print("\t%s = %s\n", cfg.full_key(), cfg.value);

    if (cfg.children)
    {
	WvHConfDict::Iter i(*cfg.children);
	for (i.rewind(); i.next(); )
	    dumpcfg(*i, s);
    }
}


int main()
{
    WvLog log("hconftest", WvLog::Info);
    
    {
	log("-- Key test begins\n");
	
	WvHConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
	WvHConfKey key2(key), key3(key, 5), key4(key, 900);
	log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    }
    
    {
	log("-- Basic config test begins\n");
	
	WvHConf cfg;
	cfg.set("/foo/blah/weasels", "chickens");
	
	cfg["foo"]["pah"]["meatballs"] = 6;
	
	WvHConf &x = cfg["snort/fish/munchkins"];
	x.set("big/bad/weasels", 7);
	
	log("Config dump:\n");
	dumpcfg(cfg, *wvcon);
    }
    
    {
	log("-- Inheritence test begins\n");
	
	WvHConf cfg, *h;
	
	cfg.set("/default/users/*/comment", "defuser comment");
	cfg.set("/default/users/bob/comment", "defbob comment");
	
	log("Old comment settings are: %s/%s\n",
	    cfg["/users/randomperson/comment"], cfg["/users/bob/comment"]);
	
	cfg["/users"].defaults = &cfg["/default/users"];
	
	h = cfg["/users/randomperson/comment"].find_default();
	log("Default for randomperson: '%s'\n", h ? *h : WvString("NONE"));
	
	h = cfg["/users/bob/comment"].find_default();
	log("Default for bob: '%s'\n", h ? *h : WvString("NONE"));
	
	log("New comment settings are: %s/%s\n",
	    cfg["/users/noperson/comment"], cfg["/users/bob/comment"]);
	
	log("Config dump 2:\n");
	dumpcfg(cfg, *wvcon);
    }
    
    return 0;
}
