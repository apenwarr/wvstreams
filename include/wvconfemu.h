/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#ifndef __WVCONFEMU_H
#define __WVCONFEMU_H

#if 0

#include "uniconfiter.h"
#include "unievents.h"

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
