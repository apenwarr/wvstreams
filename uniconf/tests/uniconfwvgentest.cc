#include "wvconf.h"
#include "uniwvconfgen.h"
#include "uniconfroot.h"
#include "wvlog.h"

WvLog mylog("WvGenTest");

int main()
{
    WvConf *_cfg = new WvConf("../../configfile/tests/testfile.ini");
    WvConf &cfg = *_cfg;
    const char *test_str;

    if (!cfg.isok())
    {
        mylog("Could not load config file.\n");
        return -1;
    }

    mylog("Starting with WvConf...\n");
    wvcon->print("\n");

    test_str = cfg.get("intl", "sLanguage", "**NOT THERE**");
    mylog("[intl]sLanguage = %s\n", test_str);
    mylog("[blah]bork = %s\n", cfg.get("blah", "bork"));
    mylog("[blah]default = %s\n", cfg.get("blah", "default", 
            "Nothing to see here... move on along..."));


    wvcon->print("\n");

    if (!cfg.isok())
    {
        mylog("Config file is now borked?\n");
        return -1;
    }

    WvConfigSection *sect = cfg["intl"];
    if (sect)
    {
        mylog("Iterating...\n");        

        WvConfigSection::Iter i(*sect);
        for (i.rewind(); i.next();)
        {
            mylog("Item: %s\n", i->name);
        }
    }

    wvcon->print("\n");
    mylog("Now testing uniconf wrapper.\n");
    wvcon->print("\n");

    UniConfRoot root(new UniWvConfGen(_cfg));

    mylog("[intl]sLanguage = %s\n", root["intl"]["sLanguage"].getme());
    mylog("[blah]bork = %s\n", root["blah"]["bork"].getme());
    mylog("[blah]default = %s\n", root["blah"]["default"].getme(
            "Nothing to see here... move on along..."));

    wvcon->print("\n");
    mylog("Iterating...\n");

    UniConfRoot::Iter i(root["intl"]);
    for (i.rewind(); i.next();)
    {
        mylog("Item: %s\n", i->key());
    }

    wvcon->print("\n");
    mylog("Now testing children / sets with uniconf wrapper.\n");
    wvcon->print("\n");


    root["Silly"]["Rabbit"].setme("Trix are for kids!");
    mylog("Setting: [Silly]Rabbit = Trix are for kids!\n");
    mylog("Reading: [Silly]Rabbit = %s\n", cfg.get("Silly", "Rabbit"));

    if (root["Blah"].haschildren())
        mylog("[Blah] has children!\n");
    if (root["Silly"].haschildren())
        mylog("[Silly] has children!\n");

    mylog("Removing [Silly]\n");
    root["Silly"].remove();

    if (root["Silly"].haschildren())
        mylog("[Silly] has children!\n");
    else
        mylog("[Silly] doesn't have children!\n");


    return 0;
}
