#include "wvtest.h"
#include "wvstring.h"
#include "wvconfemu.h"
#include "unitempgen.h"

WVTEST_MAIN("set and get")
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

WVTEST_MAIN("delete with empty key name")
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
    WvConfigSectionEmu *sect = cfg[section];
    WVPASS(sect != NULL);
    WvConfigEntryListEmu::Iter i(*sect);
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
    WvConfigSectionEmu *sect = cfg[section];
    WVPASS(sect != NULL);
    WvConfigEntryListEmu::Iter i(*sect);
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

WVTEST_MAIN("delete with NULL")
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

    WvConfigSectionEmu *sect = cfg[section];
    WVPASS(sect != NULL);

    WvConfigEntryListEmu::Iter i(*sect);
    for (i.rewind(); i.next(); )
        WVFAIL(strcmp(i->name, entry) == 0);
}

WVTEST_MAIN("delete with empty string")
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

    WvConfigSectionEmu *sect = cfg[section];
    WVPASS(sect != NULL);

    WvConfigEntryListEmu::Iter i(*sect);
    for (i.rewind(); i.next(); )
        WVFAIL(strcmp(i->name, entry) == 0);
}
