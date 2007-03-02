#include "uniconf.h"

int main()
{
    uniconf_t uni;

    uni = uniconf_init("temp:");

    uniconf_set(uni, "foo", "bar");
    printf("foo = \"%s\"\n", uniconf_get(uni, "foo"));

    uniconf_set(uni, "foo", "baz");
    printf("foo = \"%s\"\n", uniconf_get(uni, "foo"));

    uniconf_free(uni);
}


