#include "wvtest.h"
#include "wvhashtable.h"
#include "wvstring.h"

// START first old-style definition
class AutoFreeTest
{
public:
    AutoFreeTest ()
    {
//        fprintf (stderr, "AutoFreeTest created.\n");
    };
    ~AutoFreeTest ()
    {
//        fprintf (stderr, "AutoFreeTest deleted.\n");
    };
};
// END first old-style definition

WVTEST_MAIN("old-style")
{
    {
        WvMap<WvString, WvString> map(5);
        map.add ("foo", "bar");
        WVPASS(*map.find("foo") == "bar");
        map.remove("foo");
        WVFAIL(map.find("foo"));
    }

    // Iterator test
    {
        WvMap<WvString, WvString> map(5);
        map.add("meaw", "death");
        map.add("dog", "cow");
        map.add("star", "trek");
        map.add("star", "office");

        WvMap<WvString, WvString>::Iter iter(map);
        char *key[4] = {"meaw", "star", "star", "dog"};
        char *data[4] = {"death", "trek", "office", "cow"};
        int i = 0;
        for (iter.rewind(); iter.next(); i++)
        {
            WVPASS(key[i] == iter->key);
            WVPASS(data[i] == iter->data);
        }
    }

    // Check that auto_free does nothing for objects
    {
        WvMap<WvString, WvString> objfreemap (5);
        objfreemap.add ("foo", "bar", true);
        WVPASS(*objfreemap.find("foo") == "bar");
        objfreemap.remove("foo");
        WVFAIL(objfreemap.find("foo"));
    }
    {
        WvMap<WvString, AutoFreeTest*> freemap(5);
        freemap.add("moo", new AutoFreeTest(), true);
        WVPASS(freemap.find("moo"));
        freemap.remove("moo");
        WVFAIL(freemap.find("moo"));
    }

    // check if sorting confuses auto_free 
    // a'la auto_ptr's in standard containers
    {
        WvMap<WvString, AutoFreeTest*> freemap(5);
        char *keys[4] = {"meaw", "star", "star", "dog"};
        freemap.add("meaw", new AutoFreeTest(), true);
        freemap.add("dog", new AutoFreeTest(), true);
        freemap.add("star", new AutoFreeTest(), true);
        freemap.add("star", new AutoFreeTest(), true);

        WvMap<WvString, AutoFreeTest*>::Iter j(freemap);
        int i = 0;
        for (j.rewind(); j.next(); i++)
        {
            if (!WVPASS(j->key == keys[i]))
                printf("   because [%s] != [%s]", j->key.cstr(), keys[i]);
        }
    }
}

