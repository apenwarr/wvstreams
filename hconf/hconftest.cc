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
    WvHConf *parent;
    WvString name;
    WvString value;
    WvHConfDict *subkeys;
    
    WvHConf();
    WvHConf(WvHConf *_parent, const WvString &_name);
    ~WvHConf();
    
    WvHConf *top();
    WvHConfKey full_key() const;
    
    WvHConf *find(const WvHConfKey &key);
    WvHConf *find_make(const WvHConfKey &key);
    WvHConf &operator[](const WvHConfKey &key) { return *find_make(key); }
    
    WvString &get(const WvHConfKey &key)
        { return find_make(key)->value; }
    
    void set(const WvHConfKey &key, const WvString &value)
        { find_make(key)->value = value; }
    WvString &operator= (const WvString &s) { return value = s; }
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
    subkeys = NULL;
}


WvHConf::WvHConf(WvHConf *_parent, const WvString &_name)
    : name(_name)
{
    parent = _parent;
    subkeys = NULL;
}


WvHConf::~WvHConf()
{
    if (subkeys)
	delete subkeys;
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
WvHConf *WvHConf::find(const WvHConfKey &subkey)
{
    if (subkey.isempty())
	return this;
    if (!subkeys)
	return NULL;
    
    WvHConf *h = (*subkeys)[*subkey.first()];
    if (!h)
	return NULL;
    else
	return h->find(subkey.skip(1));
}


// find a key in the subtree.  If it doesn't already exist, create it.
WvHConf *WvHConf::find_make(const WvHConfKey &subkey)
{
    if (subkey.isempty())
	return this;
    if (!subkeys)
	subkeys = new WvHConfDict(10);
    
    WvHConf *h = (*subkeys)[*subkey.first()];
    
    if (h)
	return h->find_make(subkey.skip(1));
    else
    {
	// create a subkey, then call it recursively.
	WvHConf *child = new WvHConf(this, *subkey.first());
	subkeys->add(child, true);
	return child->find_make(subkey.skip(1));
    }
}


void dumpcfg(WvHConf &cfg, WvStream &s)
{
    if (!!cfg.value)
	s.print("%s = %s\n", cfg.full_key(), cfg.value);

    if (cfg.subkeys)
    {
	WvHConfDict::Iter i(*cfg.subkeys);
	for (i.rewind(); i.next(); )
	    dumpcfg(*i, s);
    }
}


int main()
{
    WvLog log("hconftest", WvLog::Info);
    WvHConf cfg;
    
    WvHConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
    WvHConfKey key2(key), key3(key, 5), key4(key, 900);
    
    log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    
    cfg.set("/foo/blah/weasels", "chickens");

    cfg["foo"]["pah"]["meatballs"] = 6;

    WvHConf &x = cfg["snort/fish/munchkins"];
    x.set("big/bad/weasels", 7);
    
    dumpcfg(cfg, *wvcon);
    
    return 0;
}
