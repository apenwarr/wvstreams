#include "uniconfroot.h"

int main()
{
    UniConfRoot root("temp:");

    root["foo"].set("bar");
    printf("foo = \"%s\"\n", root["foo"].get().cstr());

    root["foo"].set("baz");
    printf("foo = \"%s\"\n", root["foo"].get().cstr());
}
