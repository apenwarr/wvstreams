#include "wvtest.h"
#include "uniconftree.h"

WVTEST_MAIN("univaluetree basics")
{
    UniConfValueTree t(NULL, "key", "value");
    WVPASSEQ(t.value(), "value");
    WVFAIL(t.haschildren());
    UniConfValueTree t2(&t, "key2", "value2");
    WVPASS(t.haschildren());
    WVFAIL(t2.haschildren());
}
