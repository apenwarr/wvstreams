#include "uniconfroot.h"

int main()
{
    UniConfRoot root("temp:");

    root["foo"].setme("bar");
    printf("foo = \"%s\"\n", root["foo"].getme().cstr());

    root["foo"].setme("baz");
    printf("foo = \"%s\"\n", root["foo"].getme().cstr());

    UniConfRoot r("temp:");

    UniConf uu;

    UniConfKey k;

    printf("root: %d uu: %d k: %d\n",
        (int)sizeof(r), (int)sizeof(uu), (int)sizeof(k));
}
