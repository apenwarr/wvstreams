#include "wvtest.h"
#include "unimountgen.h"
#include "uniconf.h"
#include "unitempgen.h"

WVTEST_MAIN("mountgen basics")
{
    UniMountGen g;
    WVFAIL(g.haschildren("/"));
    
    g.mount("/", "null:", true);
    WVFAIL(g.haschildren("/"));
}

WVTEST_MAIN("mounting multiple generators")
{
   //FIXME: We need a reliable test for this 
}
