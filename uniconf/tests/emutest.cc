/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the WvConf emulation in UniConf.
 */

#define USE_WVCONFEMU

#include "uniconfroot.h"
#include "wvconfemu.h"
#include "wvlog.h"

int main()
{
    bool c1 = false, c2 = false, c3 = false;
    WvLog log("emutest", WvLog::Info);
#ifdef USE_WVCONFEMU
    UniConfRoot uniconf("ini:test2.ini.new");
    WvConf cfg(uniconf);
#else
    WvConf cfg("test2.ini.new");
#endif

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

    // not interesting
    log("setting [Neener]Bobber=50\n");
    cfg.setint("Neener", "Bobber", 50);
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    assert(!c1 && !c2 && !c3);
    
    // set to same value - no change event
    log("setting [Users]webmaster=NOLOGIN\n");
    cfg.set("Users", "webmaster", "NOLOGIN");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    assert(!c1 && !c2 && !c3);
    
    // should set c1
    log("setting [users]Wimp=hello\n");
    cfg.set("users", "Wimp", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    assert(c1 && !c2 && !c3);
    
    // should set c2
    log("setting [groups]bob=hello\n");
    cfg.set("groups", "bob", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    assert(c1 && c2 && !c3);
    
    // should set c3
    log("setting [users]bob=hello\n");
    cfg.set("users", "bob", "hello");
    log("ChangeBools: %s/%s/%s\n", c1, c2, c3);
    assert(c1 && c2 && c3);
  
    return 0;
}
