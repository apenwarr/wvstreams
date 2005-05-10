#include "wvtest.h"
#include "wvstring.h"
#include "wvconfemu.h"
#include "uniinigen.h"
#include <stdio.h>

WVTEST_MAIN("wvconfemu set and get")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf);
    WvString section = "TestSection", entry = "TestEntry",
        value = "TestValue", notValue = "NotTestValue",
        notSection = "No Such Section", notEntry = "No Such Entry";

    // get existing key
    cfg.set(section, entry, value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    // get nonexistant key
    WVPASS(strcmp(cfg.get(section, notEntry, notValue), notValue) == 0);
    WVPASS(strcmp(cfg.get(notSection, entry, notValue), notValue) == 0);
}

WVTEST_MAIN("wvconfemu delete with empty key name")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf);
    WvString section = "TestSection", entry = "TestEntry",
        value = "TestValue", notValue = "NotTestValue";
    
    WVPASS(strcmp(cfg.get(section, entry, notValue), notValue) == 0);

    cfg.set(section, entry, value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    cfg.set(section, "", value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    {
    WvConfigSection *sect = cfg[section];
    WVPASS(sect != NULL);
    WvConfigEntryList::Iter i(*sect);
    int count = 0;
    for (i.rewind(); i.next(); )
    {
        WVFAIL(strcmp(i->value, value) != 0);
        WVFAIL(strcmp(i->name, entry) != 0);
        ++count;
    }
    WVPASS(count == 1);
    }

    cfg.set(section, "", "");
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    {
    WvConfigSection *sect = cfg[section];
    WVPASS(sect != NULL);
    WvConfigEntryList::Iter i(*sect);
    int count = 0;
    for (i.rewind(); i.next(); )
    {
        WVFAIL(strcmp(i->value, value) != 0);
        WVFAIL(strcmp(i->name, entry) != 0);
        ++count;
    }
    WVPASS(count==1);
    }
}

WVTEST_MAIN("wvconfemu delete with NULL")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf);
    WvString section = "TestSection", entry = "TestEntry",
        value = "TestValue", notValue = "NotTestValue";
    
    cfg.set(section, entry, value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    cfg.set(section, entry, NULL);
    WVPASS(strcmp(cfg.get(section, entry, notValue), notValue) == 0);

    WvConfigSection *sect = cfg[section];
    WVPASS(sect != NULL);

    WvConfigEntryList::Iter i(*sect);
    for (i.rewind(); i.next(); )
        WVFAIL(strcmp(i->name, entry) == 0);
}

WVTEST_MAIN("wvconfemu delete with empty string")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf);
    WvString section = "TestSection", entry = "TestEntry",
        value = "TestValue", notValue = "NotTestValue";
    
    cfg.set(section, entry, value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);

    cfg.set(section, entry, "");
    WVPASS(strcmp(cfg.get(section, entry, notValue), notValue) == 0);

    WvConfigSection *sect = cfg[section];
    WVPASS(sect != NULL);

    WvConfigEntryList::Iter i(*sect);
    for (i.rewind(); i.next(); )
        WVFAIL(strcmp(i->name, entry) == 0);
}

WVTEST_MAIN("wvconfemu iterating while not mounted at root of UniConf tree")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf["/branch"]);
    WvString section = "TestSection", entry = "TestEntry",
        value = "TestValue", notValue = "NotTestValue";
        
    cfg.set(section, entry, value);
    WVPASS(strcmp(cfg.get(section, entry, notValue), value) == 0);
    
    WvConfigSection *sect = cfg[section];
    WvConfigSection::Iter i(*sect);
    for (i.rewind(); sect && i.next(); )
    {
        printf("name: %s\n", i->name.cstr());
        printf("value: %s\n", i->value.cstr());
        WVPASS(strcmp(i->name, entry) == 0);
    }

#if 0
    UniConfKey myroot("/root");
    UniConf myconf(uniconf[myroot]["/foo/bar/baz"]);
    fprintf(stderr, "from root: %s\n", myconf.fullkey().cstr());
    fprintf(stderr, "from myroot: %s\n", myconf.fullkey(myroot).cstr());
    assert(false);
#endif
}

#if 0
WVTEST_MAIN("Multiple Generators mounted on the Uniconf")
{
    {                       
    UniTempGen *tmp1 = new UniTempGen();
    UniTempGen *tmp2 = new UniTempGen();
    UniConfRoot cfg1(tmp1), cfg2(tmp2);
    UniConf uniconf(cfg1);
    WvConfEmu cfg(uniconf);

    uniconf["foo"].mountgen(tmp1);
    uniconf["foo/bar"].mountgen(tmp2);
    cfg2.setmeint(1);
    WVPASS(uniconf.xgetint("foo/bar", 0));
    WvConfigSection *sect = cfg["foo/bar"];
    WVPASS(sect);
    }
}
#endif

// see bug 9664 and bug 9769
WVTEST_MAIN("UniRetryGen not ready")
{
    UniConfRoot cfg("temp:");

    // emulate bug 9769
    class UniBuggyGen
        : public UniTempGen
    {
    public:
        UniBuggyGen()
            : UniTempGen()
            {
            }
        virtual bool exists(const UniConfKey &)
            {
                return false;
            }
    };

    cfg["foo"].setme("bar");
    cfg["retrygen"].mountgen(new UniBuggyGen());
    cfg["baz"].setme("blah");
    WvConfEmu conf(cfg);

    WvConfEmu::Iter emuiter(conf);
    for (emuiter.rewind(); emuiter.next();)
    {
        WvConfigSection &emusect = *emuiter;
        WVPASS(emuiter.ptr());
        WvConfigEntry *entry = emusect["foo"];
        WVFAIL(entry);
    } 
}

WVTEST_MAIN("Editing while iterating")
{
    UniConfRoot uniconf("temp:");
    uniconf["section"].xset("ICRASH/24", "crash1");
    uniconf["section"].xset("UCRASH/MeCRASH", "crash2");
    uniconf["section"].xset("WECRASH", "crash3");

    WvConfEmu cfg(uniconf);

    WvConfigSection *sect = cfg["section"];
    if (WVPASS(sect))
    {
        WvConfigEntryList::Iter i(*sect);
        for (i.rewind(); i.next();)
        {
            WVPASS(cfg.get("section", i->name, ""));
            cfg.set("section", i->name, NULL);
            i.rewind();
        }
        WVPASS("Didn't crash, or cause valgrind errors?");
    }
}

WVTEST_MAIN("wvconfemu setbool")
{
    UniConfRoot uniconf("temp:");
    WvConfEmu cfg(uniconf);
    bool c1, c2, c3;

    cfg.setint("Foo", "Blah", 1);

    cfg.add_setbool(&c1, "Foo", "");
    cfg.add_setbool(&c2, "", "Bar");
    cfg.add_setbool(&c3, "Foo", "Bar");

    c1 = false;
    c2 = false;
    c3 = false;

    cfg.setint("Chicken", "Poop", 1);
    WVPASS(!c1 && !c2 && !c3);

    // set it to the same value, no change event
    cfg.setint("Foo", "Blah", 1);
    WVPASS(!c1 && !c2 && !c3);

    cfg.setint("Foo", "Blah", 2);
    WVPASS(c1 && !c2 && !c3);
    c1 = false;

    cfg.setint("Something", "Bar", 1);
    WVPASS(!c1 && c2 && !c3);
    c2 = false;

    cfg.setint("Foo", "Bar", 1);
    WVPASS(c1 && c2 && c3);
    c3 = false;

    cfg.del_setbool(&c1, "Foo", "");
    cfg.del_setbool(&c2, "", "Bar");
    cfg.del_setbool(&c3, "Foo", "Bar");
}

WVTEST_MAIN("wvconfemu isempty")
{
    UniConfGen *unigen = new UniTempGen;
    UniConfRoot uniconf(unigen);
    WvConfEmu cfg(uniconf["cfg"]);

    cfg.setint("Montreal", "dcoombs", 1);

    WvConfigSectionEmu *sect = cfg["Montreal"];
    WVPASS(sect && !sect->isempty());

    cfg.set("Montreal", "dcoombs", NULL);
    WVPASS(!sect || sect->isempty());

    bool never_ran = true;
    if (sect)
    {
	WvConfigSectionEmu::Iter i(*sect);
	for (i.rewind(); i.next(); )
	    never_ran = false;
    }
    WVPASS(never_ran);
}
