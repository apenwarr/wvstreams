#include "wvhashtable.h"
#include "wvstring.h"

class AutoFreeTest
{
public:
    AutoFreeTest ()
    {
        fprintf (stderr, "AutoFreeTest created.\n");
    };
    ~AutoFreeTest ()
    {
        fprintf (stderr, "AutoFreeTest deleted.\n");
    };
};

int main ()
{
    WvMap<WvString, WvString> map (5);
    fprintf (stderr, "added foo = bar\n");
    map.add ("foo", "bar");
    fprintf (stderr, "looked up foo = %s\n", map.find ("foo")->cstr ());
    map.remove("foo");
    assert(!map.find("foo"));

    map.add("meaw", "death");
    map.add("dog", "cow");
    map.add("star", "trek");
    map.add("star", "office");

    *map["star"] = "fox";

    // Iterator test

    WvMap<WvString, WvString>::Iter i(map);

    for (i.rewind(); i.next(); )
    {
        fprintf (stderr, "Iter test: %s = %s\n", i->key.cstr(), i->data.cstr());
    }

    // Check that auto_free does nothing for objects

    WvMap<WvString, WvString> objfreemap (5);
    fprintf (stderr, "added foo = bar\n");
    objfreemap.add ("foo", "bar", true);
    fprintf (stderr, "looked up foo = %s\n", objfreemap.find ("foo")->cstr ());
    objfreemap.remove("foo");
    assert(!objfreemap.find("foo"));

    WvMap<WvString, AutoFreeTest*> freemap(5);
    freemap.add("moo", new AutoFreeTest(), true);
    assert(freemap.find("moo"));
    freemap.remove("moo");

    // check if sorting confuses auto_free 
    // a'la auto_ptr's in standard containers

    freemap.add("meaw", new AutoFreeTest());
    freemap.add("dog", new AutoFreeTest());
    freemap.add("star", new AutoFreeTest());
    freemap.add("star", new AutoFreeTest());

    WvMap<WvString, AutoFreeTest*>::Iter j(freemap);

    for (j.rewind(); j.next(); )
    {
        fprintf (stderr, "Iter test: %s\n", j->key.cstr());
    }

    // Const pointer

    WvMap<WvString, AutoFreeTest const *> constmap(5);
    constmap.add("moo", new AutoFreeTest(), true);
    assert(constmap.find("moo"));
    *constmap["moo"] = new AutoFreeTest();
    constmap.remove("moo");
    assert(!constmap.find("moo"));
}


