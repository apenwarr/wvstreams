#include "wvtest.h"
#include "unimountgen.h"

WVTEST_MAIN("mountgen basics")
{
    UniMountGen g;
    WVFAIL(g.haschildren("/"));
    
    g.mount("/", "null:", true);
    WVFAIL(g.haschildren("/"));
}
