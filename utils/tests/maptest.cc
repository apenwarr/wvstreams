#include "wvhashtable.h"
#include "wvstring.h"

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

    // Iterator test
    WvMap<WvString, WvString>::Iter i(map);

    for (i.rewind(); i.next(); )
        fprintf (stderr, "Iter test: %s = %s\n", i->key.cstr(), i->data.cstr());
}


