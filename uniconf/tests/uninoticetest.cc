#include "uniconfroot.h"
#include "wvcallback.h"
#include "wvstreamlist.h"

/**
 * This test is mostly for using with the uniconf daemon to see what
 * recursive/nonrecursive notifications do under different settings. Just netcat
 * the uniconf server and run sets / deletes by hand and see what notifications
 * pop up.
 *
 * Fun for the whole family!
 */

class foo
{
public:
    UniConfCallback meow;

    void cb(int a, const UniConf &moo, const UniConfKey &goo)
        { fprintf(stderr, "Moo... '%d' - goo = %s (%s)\n", a,
            goo.printable().cstr(), moo[goo].get().cstr()); }
};

int main()
{
    UniConfRoot r("tcp:localhost:4111");

    bool silly = false;
    bool nonsilly = false;
    bool silly2 = false;
    bool nonsilly2 = false;


    r["key/bob"].add_setbool(&silly, false);
    r["key"].add_setbool(&nonsilly, false);
    r["key/bob"].add_setbool(&silly2, true);
    r["key"].add_setbool(&nonsilly2, true);

    foo thing;

    r["heh"].add_callback(BoundCallback<UniConfCallback, int>
        (&thing, &foo::cb, 3));

 
    WvStreamList l;

    while (true)
    {
        l.select(-1);
        wvcon->print("(normals) key: %s, key/bob: %s\n", nonsilly,
silly);
        wvcon->print("(recurse) key: %s, key/bob: %s\n", nonsilly2,
silly2);

        silly2 = nonsilly2 = false;
        silly = nonsilly = false;
    }

    return 0;
}
