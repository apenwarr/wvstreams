#include "uniinigen.h"
#include "unireadonlygen.h"
#include "uniconfroot.h"
#include "wvfile.h"
#include "wvtest.h"

// defined in uniinigen.t.cc
extern void inigen(WvStringParm content);

WVTEST_MAIN("reading and (not)writing")
{
    inigen("[S1]\n"
           "a = b\n"
           "[{S2}]  \n"
           "c=d  \n"
           "[{S\n3}]\n"
           "e=f\n");
    UniConfRoot cfg("readonly:ini:tmp.ini");

    WVPASSEQ(cfg["S1/a"].get(), "b");
    WVPASSEQ(cfg["S2/c"].get(), "d");
    WVPASSEQ(cfg["S\n3/e"].get(), "f");

    cfg["S1/a"].set("y");
    cfg["S2/c"].set("z");

    // values shouldn't have changed
    WVPASSEQ(cfg["S1/a"].get(), "b");
    WVPASSEQ(cfg["S2/c"].get(), "d");
}
