/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the WvConf emulation in WvHConf.
 */
#if 0
# include "wvconf.h"
#else

//# include "wvconfemu.h"
#include "wvhconfini.h"
#include "wvhconfiter.h"

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
// real WvHConf objects into WvConfigSection objects so users can access
// the special ::Iter class.  It's nasty, but it works.
class WvConfigSection : public WvHConf
{
public:
    class Iter
    {
    public:
	WvHConf &h;
	WvHConf::RecursiveIter i;
	WvLink l;
	WvConfigEntry *e;
	
	Iter(WvHConf &_h)
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
    WvHConf h;
    WvString filename;
    
    WvConf(WvStringParm _filename, int _create_mode = 0666);
    ~WvConf();
    
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
    
    void maybesetint(WvStringParm section, WvStringParm entry,
		     int value);
    void maybeset(WvStringParm section, WvStringParm entry,
		  const char *value);

    void delete_section(WvStringParm section);
    
    void add_setbool(bool *b, WvStringParm section, WvStringParm entry)
        { }
    
    WvConfigSection *operator[] (WvStringParm sect)
        { return (WvConfigSection *)h.find_make(sect); }
    
    class Iter : public WvHConf::Iter
    {
    public:
	Iter(WvConf &cfg) : WvHConf::Iter(cfg.h)
	    { }
	
	WvConfigSection *ptr() const
	    { return (WvConfigSection *)WvHConf::Iter::ptr(); }
	
	WvIterStuff(WvConfigSection);
    };
};


WvConf::WvConf(WvStringParm _filename, int _create_mode)
    : filename(_filename)
{
    h.generator = new WvHConfIniFile(&h, filename);
    h.load();
}


WvConf::~WvConf()
{
    save();
}


const char *WvConf::get(WvStringParm section, WvStringParm entry,
			const char *def_val = NULL)
{
    WvHConf *res = h.find(WvHConfKey(section, entry));
    if (!res)
	return def_val;
    else
	return res->cstr();
}


void WvConf::setint(WvStringParm section, WvStringParm entry, int value)
{
    h.find_make(WvHConfKey(section, entry))->set(value);
}


void WvConf::set(WvStringParm section, WvStringParm entry, const char *value)
{
    h.find_make(WvHConfKey(section, entry))->set(value);
}


#endif

int main()
{
    bool c1 = false, c2 = false, c3 = false;
    WvLog log("emutest", WvLog::Info);
    WvConf cfg("test2.ini");
    
    cfg.zap();
    cfg.load_file("test2.ini");
    
    cfg.add_setbool(&c1, "Users", "");
    cfg.add_setbool(&c2, "", "Bob");
    cfg.add_setbool(&c3, "Users", "Bob");
    
    log("Test1a: '%s'\n", cfg.get("Users", "Webmaster", "foo"));
    log("Test1b: '%s'\n", cfg.get("Users", "Webmaster", NULL));
    log("Test2a: '%s'\n", cfg.get("Users", "Zebmaster", "foo"));
    log("Test2b: '%s'\n", cfg.get("Users", "Zebmaster", NULL));

    log("Single section dump:\n");
    WvConfigSection *sect = cfg["tunnel vision routes"];
    if (sect)
    {
	WvConfigEntryList::Iter i(*sect);
	for (i.rewind(); i.next(); )
	    log("  Found: '%s' = '%s'\n", i->name, i->value);
    }
    log("Section dump done.\n");
    
    log("All-section dump:\n");
    WvConfigSectionList::Iter i(cfg);
    for (i.rewind(); i.next(); )
    {
	WvConfigSection &sect = *i;
	log("  Section '%s'\n", sect.name);
	WvConfigSection::Iter i2(sect);
	i2.rewind(); i2.next();
	if (i2.cur())
	    log("   First entry: '%s'='%s'\n", i2->name, i2->value);
    }
    log("All-section dump done.\n");

    cfg.setint("Neener", "Bobber", 50);
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    
    cfg.set("Users", "webmaster", "NOLOGIN");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    
    cfg.set("users", "Wimp", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    
    cfg.set("groups", "bob", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    
    cfg.set("users", "bob", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
  
    return 0;
}
