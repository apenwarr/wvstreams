// This class is used to test the basic sanity of a UniConf generator.  This
// tests things that might not be strictly guaranteed by UniConf, but are true
// for all known generators.  If your generator fails these, you either need
// to rethink a bit, or put some very large warnings on your generator.

#ifndef UNICONFGEN_SANITYTEST_H
#define UNICONFGEN_SANITYTEST_H

#include "wvstring.h"
#include "wvlog.h"
#include "uniconf.h"
#include "uniconfkey.h"

class IUniConfGen;

class UniConfGenSanityTester
{
public:
    static void clear_generator(IUniConfGen *g);

    static void sanity_test(IUniConfGen *g, WvStringParm moniker);
    static void test_trailing_slashes(IUniConfGen *g, WvStringParm moniker);
    static void test_haschildren_gen(IUniConfGen *g);
    static void test_haschildren_moniker(WvStringParm moniker);
    static void test_iter_sanity(WvStringParm moniker);
    static void test_recursive_iter_sanity(WvStringParm moniker);

    // Same as the one in unicachegen.t.cc
    class CbCounter
    {
    public:
        CbCounter() : 
            cbs(0),
            log("Callback counter", WvLog::Info)
        { }
        void callback(const UniConf keyconf, const UniConfKey key) 
        { 
            log("Got callback for key '%s', value '%s'\n", 
                    keyconf[key].fullkey().printable(), 
                    keyconf[key].getme());
            cbs++;
        }
        int cbs;
        WvLog log;
    };
};

#endif

