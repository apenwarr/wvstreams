#include "uniconfroot.h"

int main()
{
    UniConfRoot root("temp:");

    root["foo"].setme("bar");
    printf("foo = \"%s\"\n", root["foo"].getme().cstr());

    root["foo"].setme("baz");
    printf("foo = \"%s\"\n", root["foo"].getme().cstr());
}
