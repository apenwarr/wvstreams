/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#ifndef __WVCONFEMU_H
#define __WVCONFEMU_H


#ifndef __WVCONF_H
#define WvConf WvConfEmu
#define WvConfigSection WvConfigSectionEmu
#define WvConfigSectionList WvConfigSectionListEmu
#define WvConfigEntry WvConfigEntryEmu
#define WvConfigEntryList WvConfigEntryListEmu
#endif


#include "uniconfroot.h"
#include "wvstream.h"


class WvConfEmu;
class WvConfigEntryEmu;
class WvConfigSectionEmu;

typedef WvConfEmu WvConfigSectionListEmu;
typedef WvConfigSectionEmu WvConfigEntryListEmu;


class WvConfigEntryEmu
{
public:
    const WvString name;
    const WvString value;
    WvConfigEntryEmu(WvStringParm _name, WvStringParm _value):
	name(_name), value(_value)
    {}
};


class WvConfigSectionEmu
{
private:
    const UniConf uniconf;
public:
    const WvString name;
    WvConfigSectionEmu(const UniConf& _uniconf, WvStringParm _name):
	uniconf(_uniconf), name(_name)
    {}
    class Iter;
    friend Iter;
};


DeclareWvDict(WvConfigSectionEmu, WvString, name);


class WvConfigSectionEmu::Iter
{
private:
    UniConf::Iter iter;
    WvLink link;
    WvConfigEntryEmu* entry;
public:
    Iter(WvConfigSectionEmu& _sect):
	iter(_sect.uniconf), link(NULL, false),
	entry(NULL)
    {}
    void rewind()
    {
	iter.rewind();
    }
    WvLink *next()
    {
	if (iter.next())
	{
	    entry = new WvConfigEntryEmu(iter->key(), iter->get());
	    link.data = static_cast<void*>(entry);
	    return &link;
	}

	return NULL;
    }
    WvLink *cur()
    {
	return &link;
    }
    WvConfigEntryEmu* ptr() const
    {
	return entry;
    }
    WvIterStuff(WvConfigEntryEmu);
};


class WvConfEmu
{
private:
    struct SetBool
    {
	bool* b;
	WvString section;
	WvString key;
	SetBool(bool* _b, WvStringParm _section, WvStringParm _key):
	    b(_b), section(_section), key(_key)
	{}
    };

    const UniConf uniconf;
    WvConfigSectionEmuDict sections;
    bool hold;
    WvList<SetBool> setbools;

    void notify(const UniConf &_uni, const UniConfKey &_key)
    {
	WvList<SetBool>::Iter i(setbools);
	WvString section(_key.first());
	WvString key(_key.removefirst());

	if (hold)
	    return;

	i.rewind();
	while (i.next())
	    if (((i->section && !i->section) || !strcasecmp(i->section, section))
		&& ((i->key && !i->key) || !strcasecmp(i->key, key)))
		*(i->b) = true;
    }
public:
    WvConfEmu(UniConf& _uniconf):
	uniconf(_uniconf), sections(42), hold(false)
    {
	uniconf.add_callback(this,
			     UniConfCallback(this, &WvConfEmu::notify),
			     true);
    }
    void zap()
    {
	uniconf.remove();
    }
    void load_file(WvStringParm filename)
    {
	UniConfRoot new_uniconf(WvString("ini:%s", filename));

	hold = true;
	new_uniconf.copy(uniconf, true);
	hold = false;
    }
    WvConfigSectionEmu *operator[] (WvStringParm sect)
    {
	WvConfigSectionEmu* section = sections[sect];

	if (!section && uniconf[sect].exists())
	{
	    section = new WvConfigSectionEmu(uniconf[sect], sect);
	    sections.add(section, true);
	}

	return section;
    }
    void add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
    {
	WvList<SetBool>::Iter i(setbools);

	i.rewind();
	while (i.next())
	{
	    if (i->b == b
		&& i->section == _section
		&& i->key == _key)
		return;
	}

	setbools.append(new SetBool(b, _section, _key), true);
    }
    const char *get(WvStringParm section, WvStringParm entry,
		    const char *def_val = NULL)
    {
	return uniconf[section][entry].get(def_val);
    }
    void setint(WvStringParm section, WvStringParm entry, int value)
    {
	uniconf[section][entry].setint(value);
    }
    void set(WvStringParm section, WvStringParm entry,
	     const char *value)
    {
	uniconf[section][entry].set(value);
    }

    class Iter;
    friend Iter;
};


class WvConfEmu::Iter
{
    WvConfEmu& conf;
    UniConf::Iter iter;
    WvLink link;
public:
    Iter(WvConfEmu& _conf):
	conf(_conf), iter(conf.uniconf), link(NULL, false)
    {}
    void rewind()
    {
	iter.rewind();
    }
    WvLink *next()
    {
	if (iter.next())
	{
	    link.data = static_cast<void*>(conf[iter->key()]);
	    return &link;
	}

	return NULL;
    }
    WvConfigSectionEmu* ptr() const
    {
	return conf[iter->key()];
    }
    WvIterStuff(WvConfigSectionEmu);
};


#if 0

class WvConfigEntryEmu;
class WvConfigSectionEmu;
class WvConfEmu;

typedef WvConfigEntryEmu   WvConfigEntry;
typedef WvConfigSectionEmu WvConfigSection;
typedef WvConfEmu          WvConf;

#define WvConf WvConfEmu
#define WvConfigSection WvConfigSectionEmu
#define WvConfigEntry WvConfigEntryEmu

typedef WvConfigSection WvConfigEntryList;
typedef WvConf          WvConfigSectionList;


class WvConfigEntry
{
public:
    WvString name, value;
    
    WvConfigEntry(WvStringParm _name, WvStringParm _value)
	: name(_name), value(_value)
	{ }
};


// AVERY IS INCREDIBLY EVIL ALARM!!
// We never actually create any WvConfigSection objects, so we mustn't have
// any virtual functions or member variables.  Instead, we just typecast
// real UniConf objects into WvConfigSection objects so users can access
// the special ::Iter class.  It's nasty, but it works.
class WvConfigSection : public UniConf
{
public:
    WvString name;


    class Iter
    {
    public:
	UniConf &h;
	UniConf::RecursiveIter i;
	WvLink l;
	WvConfigEntry *e;
	
	Iter(UniConf &_h)
	    : h(_h), i(h), l(NULL, false)
	    { e = NULL; }
	
	~Iter()
	    { deadlink(); }
	
	WvLink *deadlink()
	{
	    if (e) delete e;
	    l.data = e = NULL;
	    return NULL;
	}
	
	WvLink *mklink()
	{
	    if (e) delete e;
	    l.data = e = new WvConfigEntry(i->full_key(&h), i->printable());
	    return &l;
	}
	
	void rewind()   { i.rewind(); deadlink(); }
	WvLink *cur()   { return i.cur() ? mklink() : deadlink(); }
	
	WvLink *next()
	{
	    while (i.next() && !*i)
		;
	    return cur(); 
	}
	
	WvConfigEntry *ptr() const { return e; }
	
	WvIterStuff(WvConfigEntry);
    };
};



class WvConf
{
public:
    UniConf h;
    UniConfNotifier notifier;
    UniConfEvents ev;
    WvString filename;
    
    WvConf(WvStringParm _filename, int _create_mode = 0666);
    ~WvConf();
    
    // for transparency, we can pass a WvConf object as a parameter to
    // someone that expects a UniConf object, and this typecast operator
    // will convert them automatically.
    operator UniConf & ()
        { return h; }
    
    bool isok() const
        { return true; }
    bool isclean() const
        { return true; /* FIXME */ }
    
    void zap()
        { /* FIXME */ }
    
    void load_file() // append the contents of the real config file
        { h.load(); }
    void load_file(WvStringParm _filename) // append any config file
        { h.load(); }

    void save(WvStringParm _filename)
        { h.save(); }
    void save()
        { h.save(); }
    void flush()
        { save(); }

    int getint(WvStringParm section, WvStringParm entry, int def_val);
    
    const char *get(WvStringParm section, WvStringParm entry,
		    const char *def_val = NULL);
    WvString getraw(WvString wvconfstr, int &parse_error);

    int fuzzy_getint(WvStringList &sect, WvStringParm entry,
		  int def_val);
    const char *fuzzy_get(WvStringList &sect, WvStringParm entry,
			  const char *def_val = NULL);

    int fuzzy_getint(WvStringList &sect, WvStringList &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList & sect, WvStringList & ent,
			  const char *def_val = NULL);

    void setint(WvStringParm section, WvStringParm entry, int value);
    void set(WvStringParm section, WvStringParm entry,
	     const char *value);
    void setraw(WvString wvconfstr, const char *&value, int &parse_error);
    
    void delete_section(WvStringParm section);
    
    void add_setbool(bool *b, WvStringParm section, WvStringParm entry);
    void del_setbool(bool *b, WvStringParm section, WvStringParm entry);
    void run_all_callbacks()
        { notifier.run(); }
    
    WvConfigSection *operator[] (WvStringParm sect)
        { return (WvConfigSection *)h.find_make(sect); }
    
    class Iter : public UniConf::Iter
    {
    public:
	Iter(WvConf &cfg) : UniConf::Iter(cfg.h)
	    { }
	
	WvConfigSection *ptr() const
	    { return (WvConfigSection *)UniConf::Iter::ptr(); }
	
	WvIterStuff(WvConfigSection);
    };
};

#endif


#endif // __WVCONFEMU_H
