#include "uniinigen.h"
#include "unireadonlygen.h"
#include "uniconfroot.h"
#include "wvfile.h"
#include "wvtest.h"

// NOTE: UniReadOnlyGen violates the "sensible" UniConf semantics tested for
// in uniconfgen-sanitytest.h, since setting a value doesn't work.
// FIXME: Perhaps the tests should take this into account

// defined in uniinigen.t.cc
extern WvString inigen(WvStringParm content);

WVTEST_MAIN("reading and (not)writing")
{
    WvString inifile = inigen("[S1]\n"
           "a = b\n"
           "[{S2}]  \n"
           "c=d  \n"
           "[{S\n3}]\n"
           "e=f\n");
    WvString mon("readonly:ini:%s", inifile);
    IUniConfGen *gen = wvcreate<IUniConfGen>(mon);
    UniConfRoot cfg((UniConfGen*)gen);

    WVPASSEQ(cfg["S1/a"].getme(), "b");
    WVPASSEQ(cfg["S2/c"].getme(), "d");
    WVPASSEQ(cfg["S\n3/e"].getme(), "f");

    cfg["S1/a"].setme("y");
    cfg["S2/c"].setme("z");

    // values shouldn't have changed
    WVPASSEQ(cfg["S1/a"].getme(), "b");
    WVPASSEQ(cfg["S2/c"].getme(), "d");

    ::unlink(inifile);
}
