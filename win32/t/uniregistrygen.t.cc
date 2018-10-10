#include "uniconfroot.h"
#include "wvlinkerhack.h"
#include "wvtest.h"

WV_LINK_TO(UniRegistryGen);


WVTEST_MAIN("uniregistry")
{
    UniConfRoot uni("registry:HKEY_CURRENT_USER");
    WVPASS(!!uni.xget("/Control Panel/Keyboard/KeyboardDelay"));
    
    UniConf::RecursiveIter i(uni["Software/Microsoft/Windows"]);
    int count;
    for (i.rewind(), count = 0; i.next() && count < 10; count++)
	printf("Key '%s' = '%s'\n", i->key().cstr(), i->getme().cstr());
}
