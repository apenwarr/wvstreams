/*
* Worldvisions Weaver Software:
*   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
*
* Plays with WvConfigSectionList (without the WvConf file interface wrapper)
*
*/

#include "wvconf.h"

int main()
{
    WvConfigSectionList sectlist;
    sectlist.append(new WvConfigSection("Hello"), true);
    sectlist.append(new WvConfigSection("Hellooo"), true);
    sectlist.append(new WvConfigSection("\n# test\n"), true);
    sectlist.append(new WvConfigSection("aaaa"), true);
    sectlist.append(new WvConfigSection("bbbb"), true);

    WvConfigSectionList::Iter zzz(sectlist);
    zzz.rewind();zzz.next();
    WvConfigSection *sect = &*zzz;

    if(!sect) {
	printf("crap\n");
	return 0;
    }

    sect->set("suck", "blah");
    sect->set("buck", "more blah");
    sect->set("luck", "even more");
    sect->set("duck", "bored now");


    zzz.rewind(); zzz.next();
    sect = &*zzz;

    WvConfigEntry *luck = (*zzz)["luck"];

    if(!!luck)
    {
        WvString value = luck->value;
	value = WvString("DIRTIED! [%s]", value);
	luck->value = value;
        printf("Lucky me [%s]\n", luck->value.edit());
    }
    else
        zzz->quick_set("weeeee", "waaaaah");

    zzz->quick_set("weeeee", "ARGH");

    
    printf("--- list everything ---\n");
    for(zzz.rewind(); zzz.next(); )
    {
        printf("[%s]\n", zzz->name.edit());
        zzz->dump(*wvcon);
    }
    printf("------- end list ------\n");

    return 0; 
    
}
