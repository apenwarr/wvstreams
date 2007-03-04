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

// Create a UniConfDaemon upon instantiation.  By default, the server listens
// to the given socket name, and mounts the given moniker, and doesn't do much
// else.  The server is killed and its socket is unlinked by the destructor.
class UniConfTestDaemon
{
    pid_t server_pid;
    WvString sockname;
    WvString server_moniker;
public:
    typedef WvCallback<void, WvStringParm, WvStringParm> UniConfDaemonServerCb;

    // A default server callback that doesn't do anything interesting, just
    // mounts the given moniker on the given socket.
    static void boring_server_cb(WvStringParm sockname, 
            WvStringParm server_moniker);

    // Just like the boring server, but increments a key every time through
    // its select loop, to generate some notifications.
    static void autoinc_server_cb(WvStringParm sockname, 
            WvStringParm server_moniker);

    // server_cb is the main body of the server.  It must not return.
    UniConfTestDaemon(WvStringParm _sockname, WvStringParm _server_moniker, 
        UniConfDaemonServerCb server_cb = boring_server_cb);
    ~UniConfTestDaemon();

    pid_t get_pid() { return server_pid; }
    WvString get_sockname() { return sockname; }
    WvString get_moniker() { return server_moniker; }
};

#endif

