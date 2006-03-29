#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniconfgen-sanitytest.h"

WVTEST_MAIN("UniTempGen Sanity Test")
{
    UniTempGen *gen = new UniTempGen();
    UniConfGenSanityTester::sanity_test(gen, "temp:");
    WVRELEASE(gen);
}

// FIXME: could test lots more stuff here, or rather in the Sanity Tester...
