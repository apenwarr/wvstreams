#include "uniconfwvgen.h"
#include "uniconf.h"
#include "wvconf.h"
#include "wvlog.h"

WvLog log("WvGenTest");

int main()
{
    WvConf cfg("../../configfile/tests/testfile.ini");
    const char *test_str;

    if (!cfg.isok())
    {
        log("Could not load config file.\n");
        return -1;
    }

    log("Starting with WvConf...\n");
    wvcon->print("\n");

    test_str = cfg.get("intl", "sLanguage", "**NOT THERE**");
    log("[intl]sLanguage = %s\n", test_str);
    log("[blah]bork = %s\n", cfg.get("blah", "bork"));
    log("[blah]default = %s\n", cfg.get("blah", "default", 
            "Nothing to see here... move on along..."));


    wvcon->print("\n");

    if (!cfg.isok())
    {
        log("Config file is now borked?\n");
        return -1;
    }

    WvConfigSection *sect = cfg["intl"];
    if (sect)
    {
        log("Iterating...\n");        

        WvConfigSection::Iter i(*sect);
        for (i.rewind(); i.next();)
        {
            log("Item: %s\n", i->name);
        }
    }

    wvcon->print("\n");
    log("Now testing uniconf wrapper.\n");
    wvcon->print("\n");

    UniConfRoot root(new UniConfWvGen(cfg));

    log("[intl]sLanguage = %s\n", root["intl"]["sLanguage"].get());
    log("[blah]bork = %s\n", root["blah"]["bork"].get());
    log("[blah]default = %s\n", root["blah"]["default"].get(
            "Nothing to see here... move on along..."));

    wvcon->print("\n");
    log("Iterating...\n");

    UniConfRoot::Iter i(root["intl"]);
    for (i.rewind(); i.next();)
    {
        log("Item: %s\n", i->key());
    }

    wvcon->print("\n");
    log("Now testing children / sets with uniconf wrapper.\n");
    wvcon->print("\n");


    root["Silly"]["Rabbit"].set("Trix are for kids!");
    log("Setting: [Silly]Rabbit = Trix are for kids!\n");
    log("Reading: [Silly]Rabbit = %s\n", cfg.get("Silly", "Rabbit"));

    if (root["Blah"].haschildren())
        log("[Blah] has children!\n");
    if (root["Silly"].haschildren())
        log("[Silly] has children!\n");

    log("Removing [Silly]\n");
    root["Silly"].zap();

    if (root["Silly"].haschildren())
        log("[Silly] has children!\n");
    else
        log("[Silly] doesn't have children!\n");


    return 0;
}
