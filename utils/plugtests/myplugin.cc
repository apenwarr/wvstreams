#include "wvplugin.h"
#include "wvfile.h"
#include <stdio.h>

void *create_test_object(WvStringParm s, IObject *obj, void *userdata)
{
    printf("create_test_object (%s)...\n", s.cstr());
    return new WvFile("/tmp/foofile", O_RDONLY);
}


static const UUID test_CID = {
    0x09674e8a, 0x74cb, 0x4b30,
    {0xb6, 0xbd, 0x5c, 0x9c, 0xcb, 0xe0, 0xd2, 0x4f}
};

static const UUID test2_CID = {
    0x09674e8a, 0x74cc, 0x4b30,
    {0xb6, 0xbd, 0x5c, 0x9c, 0xcb, 0xe0, 0xd2, 0x4f}
};

WVPLUGIN_START("sample plugin module")
    WVPLUGIN(IWvStream, "test", test_CID, create_test_object)
    WVPLUGIN(IWvStream, "test2", test2_CID, create_test_object)
WVPLUGIN_END
