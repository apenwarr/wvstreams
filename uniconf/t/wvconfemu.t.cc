#include "wvtest.h"
#include "wvstring.h"
#include "wvconfemu.h"
#include "unitempgen.h"

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
