#include "wvtest.h"
#include "uniconfroot.h"
#include "unilistgen.h"
#include "unitempgen.h"
#include "uniinigen.h"
#include <stdio.h>

WVTEST_MAIN("Testing for use with weaver")
{
    UniIniGen *inigen = new UniIniGen("./listgen.ini", 0600);
//    UniTempGen *inigen = new UniTempGen();
    UniTempGen *tmpgen = new UniTempGen();
    UniConfGenList *l = new UniConfGenList();
    l->append(inigen, false);
    l->append(tmpgen, false);
    UniListGen *unigen = new UniListGen(l);
    
    UniConfRoot uniconf(unigen);
    UniConfRoot front(inigen);
    UniConfRoot back(tmpgen);

    //should work as a normal generator(saving to ini)
    uniconf.xsetint("Monkey", 1);
    WVPASS(uniconf.xgetint("Monkey"));
    WVPASS(front.xgetint("Monkey"));
    WVFAIL(back.xgetint("Monkey"));
    
    WVFAIL(uniconf.xgetint("Banana"));
    //should read from tmpgen in background
    back.xsetint("Banana", 1);
    printf("%i\n", back.xgetint("Banana"));
    WVPASS(uniconf.xgetint("Banana"));
    WVPASS(back.xgetint("Banana"));
    
    uniconf.xsetint("Banana", 0);
    WVFAIL(uniconf.xgetint("Banana"));
    WVFAIL(front.xgetint("Banana"));
    // back should still be as it was
    printf("%i\n", back.xgetint("Banana"));
    WVPASS(back.xgetint("Banana"));
    
    WVFAIL(uniconf.xgetint("Manana"));
}
