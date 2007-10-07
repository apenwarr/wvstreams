#include "uniclientgen.h"
#include "uniconfdaemon.h"
#include "uniconf.h"
#include "uniconfroot.h"
#include "unilistgen.h"
#include "unitempgen.h"
#include "unitransactiongen.h"
#include "unitransaction.h"
#include "uniunwrapgen.h"
#include "uniwatch.h"
#include "uniconfgen-sanitytest.h"

#include "wvfile.h"
#include "wvhashtable.h"
#include "wvtest.h"
#include "wvunixsocket.h"

#include <signal.h>

WVTEST_MAIN("UniTransactionGen Sanity Test")
{
    UniTransactionGen *gen = new UniTransactionGen(new UniTempGen());
    UniConfGenSanityTester::sanity_test(gen, "transaction:temp:");
    WVRELEASE(gen);
}

static WvMap<UniConfKey, WvString> callbacks2(5);
static WvMap<UniConfKey, WvString> callbacks1(5);

static void check_callback2(const UniConf &handle,
			    const UniConfKey &key)
{
    wverr->print("Callback on key \"%s\" with value \"%s\".\n",
	   key, handle[key].getme());
    bool expected;
    WVPASS(expected = callbacks2.exists(key));
    if (expected)
    {
	WVPASSEQ(callbacks2[key], handle[key].getme());
	callbacks2.remove(key);
    }
}

static void check_callback1(const UniConf &handle,
			    const UniConfKey &key)
{
    wverr->print("Callback on key \"%s\" with value \"%s\".\n",
	   key, handle[key].getme());
    bool expected;
    WVPASS(expected = callbacks1.exists(key));
    if (expected)
    {
	WVPASSEQ(callbacks1[key], handle[key].getme());
	callbacks1.remove(key);
    }
}

static UniConfCallback callback2(check_callback2);
static UniConfCallback callback1(check_callback1);

static void check_iterator(WvMap<UniConfKey, WvString> &map,
			   const UniConf &handle)
{
    UniConf::RecursiveIter i(handle);
    for (i.rewind(); i.next();)
    {
	bool expected;
	WVPASS(expected = map.exists(i.ptr()->fullkey()));
	if (expected)
	{
	    WVPASSEQ(map[i.ptr()->fullkey()], i.ptr()->getme());
	    map.remove(i.ptr()->fullkey());
	}
    }
    WVPASS(map.isempty());
    map.zap();
}

WVTEST_MAIN("UniTransactionGen functionality test")
{
    UniTempGen *gen = new UniTempGen();
    UniConfRoot one(gen);
    UniConfRoot two(new UniTransactionGen(new UniUnwrapGen(one)));
    UniWatch mywatch2(two, callback2);

    // Test a TransactionGen with no changes.

    callbacks2.add("cfg", "Filled");
    callbacks2.add("cfg/OpenWall", "");
    callbacks2.add("cfg/OpenWall/Harden Proc", 1);
    callbacks2.add("cfg/OpenWall/Harden RLIMIT", 1);
    callbacks2.add("cfg/OpenWall/Harden Link", 1);
    callbacks2.add("cfg/OpenWall/Harden Stack", 1);

    one.xset("cfg", "Filled");
    one.xset("cfg/OpenWall/Harden Proc", 1);
    one.xset("cfg/OpenWall/Harden RLIMIT", 1);
    one.xset("cfg/OpenWall/Harden Link", 1);
    one.xset("cfg/OpenWall/Harden Stack", 1);

    WVPASS(callbacks2.isempty());
    callbacks2.zap();

    WVPASSEQ(two.xget(""), "");
    WVPASSEQ(two.xget("cfg"), "Filled");
    WVPASSEQ(two.xget("cfg/OpenWall"), "");
    WVPASSEQ(two.xget("cfg/OpenWall/Harden Proc"), "1");
    WVPASSEQ(two.xget("cfg/OpenWall/Harden RLIMIT"), "1");
    WVPASSEQ(two.xget("cfg/OpenWall/Harden Link"), "1");
    WVPASSEQ(two.xget("cfg/OpenWall/Harden Stack"), "1");
    WVPASSEQ(two.xget("cfg/foo"), WvString::null);

    callbacks2.add("cfg", "Filled");
    callbacks2.add("cfg/OpenWall", "");
    callbacks2.add("cfg/OpenWall/Harden Proc", 1);
    callbacks2.add("cfg/OpenWall/Harden RLIMIT", 1);
    callbacks2.add("cfg/OpenWall/Harden Link", 1);
    callbacks2.add("cfg/OpenWall/Harden Stack", 1);
    check_iterator(callbacks2, two);

    callbacks2.add("cfg/OpenWall/Harden Proc", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden Link", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden RLIMIT", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden Stack", WvString::null);
    callbacks2.add("cfg/OpenWall", WvString::null);
    callbacks2.add("cfg", WvString::null);
    callbacks2.add("", WvString::null);
    one.xset("", WvString::null);
    WVPASS(callbacks2.isempty());
    callbacks2.zap();


    // Test gets and callbacks on new values.

    callbacks2.add("", "");
    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);

    two.xset("cfg/Global/Have Disk", 1);

    WVPASS(callbacks2.isempty());
    callbacks2.zap();

    WVPASSEQ(two.xget(""), "");
    WVPASSEQ(two.xget("cfg"), "");
    WVPASSEQ(two.xget("cfg/Global"), "");
    WVPASSEQ(two.xget("cfg/Global/Have Disk"), "1");
    WVPASSEQ(two.xget("cfg/bar"), WvString::null);

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test that our changes override their changes.

    one.xset("cfg/Global/Have Disk", 0);
    WVPASSEQ(two.xget("cfg/Global/Have Disk"), "1");

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test that they DON'T override their changes to child keys.

    callbacks2.add("cfg/Global/Have Disk/Really Have Disk", 1);
    one.xset("cfg/Global/Have Disk/Really Have Disk", 1);
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg/Global/Have Disk/Really Have Disk"), "1");

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    callbacks2.add("cfg/Global/Have Disk/Really Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test that underlying callbacks on keys changed to !! strings 
    // that previously were ! strings and were created due to a change
    // are received.

    callbacks2.add("cfg", "exists");
    one.xset("cfg", "exists");
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg"), "exists");

    callbacks2.add("cfg", "exists");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    callbacks2.add("cfg/Global/Have Disk/Really Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test that it changes back to the empty string. Also, after r4_2,
    // test that deleting keys in the underlying generator to which we
    // are making changes other than a tree replacement succeeds in
    // causing the necessary callbacks for children of that key.

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global/Have Disk/Really Have Disk", WvString::null);
    one.xset("cfg", WvString::null);
    WVPASS(!callbacks2.exists("cfg"));
    WVPASS(!callbacks2.exists("cfg/Global/Have Disk/Really Have Disk"));
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg"), "");

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test that underlying callbacks on keys that previously were ! strings,
    // were created due to a change, and are STILL ! strings are NOT
    // received.

    one.xset("cfg/Global", "");
    WVPASSEQ(two.xget("cfg/Global"), "");

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test we don't get callbacks when we change them back, either.

    one.xset("cfg/Global", WvString::null);
    WVPASS(callbacks2.isempty());
    callbacks2.zap();

    WVPASSEQ(two.xget("cfg/Global"), "");


    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 1);
    check_iterator(callbacks2, two);

    // Test reception of callbacks due to deletions

    callbacks2.add("cfg/Global/Have Disk", WvString::null);
    callbacks2.add("cfg/Global", WvString::null);
    two.xset("cfg/Global", WvString::null);
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg/Global/Have Disk"), WvString::null);
    WVPASSEQ(two.xget("cfg/Global"), WvString::null);

    callbacks2.add("cfg", "");
    check_iterator(callbacks2, two);

    // Test non-reception of underlying callbacks on sections that will be
    // deleted by a change.
    
    one.xset("cfg/Global/Servers/FunFS", 1);
    one.xset("cfg/Global/Servers", WvString::null);
    WVPASSEQ(two.xget("cfg/Global"), WvString::null);
    WVPASSEQ(two.xget("cfg/Global/Servers"), WvString::null);
    WVPASSEQ(two.xget("cfg/Global/Servers/FunFS"), WvString::null);

    callbacks2.add("cfg", "");
    check_iterator(callbacks2, two);
    
    // Test reception of callbacks on changes to replacement sections.
    
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 0);
    two.xset("cfg/Global/Have Disk", 0);
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg/Global"), "");
    WVPASSEQ(two.xget("cfg/Global/Have Disk"), "0");

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 0);
    check_iterator(callbacks2, two);

    // Test non-creation of keys when deleting subkeys.

    two.xset("cfg/foo/bar", WvString::null);
    WVPASSEQ(two.xget("cfg/foo"), WvString::null);
    WVPASSEQ(two.xget("cfg/foo/bar"), WvString::null);

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 0);
    check_iterator(callbacks2, two);
    
    // Test that they really will be deleted, and also test reception
    // of callbacks on keys to which nothing is being done.

    callbacks2.add("cfg/foo", "");
    one.xset("cfg/foo/bar", "");
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg/foo"), "");
    WVPASSEQ(two.xget("cfg/foo/bar"), WvString::null);

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/foo", "");
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Have Disk", 0);
    check_iterator(callbacks2, two);

    // Test that refresh works.
    callbacks2.add("cfg/foo/bar", "");
    callbacks2.add("cfg/Global/Have Disk/", WvString::null);
    two.refresh();
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASSEQ(two.xget("cfg/foo/bar"), "");
    WVPASSEQ(two.xget("cfg/Global/Have Disk/"), WvString::null);

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/foo", "");
    callbacks2.add("cfg/foo/bar", "");
    callbacks2.add("cfg/Global", "");    
    check_iterator(callbacks2, two);

    // Prepare to test that commit works.
    callbacks2.add("cfg/Global", WvString::null);
    callbacks2.add("cfg/foo/bar", WvString::null);
    callbacks2.add("cfg/foo", WvString::null);
    callbacks2.add("cfg", WvString::null);
    callbacks2.add("", WvString::null);
    one.xset("", WvString::null);
    callbacks2.add("", "");
    callbacks2.add("cfg", "");
    callbacks2.add("cfg/OpenWall", "");
    callbacks2.add("cfg/OpenWall/Harden Stack", 1);
    callbacks2.add("cfg/OpenWall/Harden Link", 1);
    callbacks2.add("cfg/OpenWall/Harden FIFO", 1);    
    callbacks2.add("cfg/OpenWall/Harden Proc", 1);
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Servers", "");
    callbacks2.add("cfg/Global/Servers/Funfs", 1);
    callbacks2.add("cfg/Global/Servers/NFS", 0);
    one.xset("cfg/OpenWall/Harden Stack", 1);
    one.xset("cfg/OpenWall/Harden Link", 1);
    one.xset("cfg/OpenWall/Harden FIFO", 1);
    one.xset("cfg/OpenWall/Harden Proc", 1);
    one.xset("cfg/Global/Servers/Funfs", 1);
    one.xset("cfg/Global/Servers/NFS", 0);
    callbacks2.add("cfg/OpenWall/Harden Stack", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden Link", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden FIFO", WvString::null);
    callbacks2.add("cfg/OpenWall/Harden Proc", WvString::null);
    callbacks2.add("cfg/OpenWall", WvString::null);
    two.xset("cfg/OpenWall", WvString::null);
    callbacks2.add("cfg/OpenWall", "");
    callbacks2.add("cfg/OpenWall/Harden Stack", 0);
    callbacks2.add("cfg/OpenWall/Harden FIFO", "");    
    callbacks2.add("cfg/OpenWall/Harden Proc", 1);
    callbacks2.add("cfg/Global/Servers/FunFS", 0);
    two.xset("cfg/OpenWall/Harden Stack", 0);
    two.xset("cfg/OpenWall/Harden FIFO", "");
    two.xset("cfg/OpenWall/Harden Proc", 1);
    two.xset("cfg/Global/Servers/FunFS", 0);
    two.xset("cfg/Global/Servers/NFS", 0);
    UniWatch mywatch1(one, callback1);
    callbacks1.add("cfg/OpenWall/Harden Link", WvString::null);
    callbacks1.add("cfg/OpenWall/Harden Stack", 0);
    callbacks1.add("cfg/OpenWall/Harden FIFO", "");
    callbacks1.add("cfg/Global/Servers/FunFS", 0);
    // Here we go.
    two.commit();
    WVPASS(callbacks2.isempty());
    callbacks2.zap();
    WVPASS(callbacks1.isempty());
    callbacks1.zap();

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/OpenWall", "");
    callbacks2.add("cfg/OpenWall/Harden Stack", 0);
    callbacks2.add("cfg/OpenWall/Harden FIFO", "");    
    callbacks2.add("cfg/OpenWall/Harden Proc", 1);
    callbacks2.add("cfg/Global", "");
    callbacks2.add("cfg/Global/Servers", "");
    callbacks2.add("cfg/Global/Servers/Funfs", 0);
    callbacks2.add("cfg/Global/Servers/NFS", 0);
    check_iterator(callbacks2, two);

    callbacks1.add("cfg", "");
    callbacks1.add("cfg/OpenWall", "");
    callbacks1.add("cfg/OpenWall/Harden Stack", 0);
    callbacks1.add("cfg/OpenWall/Harden FIFO", "");    
    callbacks1.add("cfg/OpenWall/Harden Proc", 1);
    callbacks1.add("cfg/Global", "");
    callbacks1.add("cfg/Global/Servers", "");
    callbacks1.add("cfg/Global/Servers/Funfs", 0);
    callbacks1.add("cfg/Global/Servers/NFS", 0);
    check_iterator(callbacks1, one);
}

// Test that UniTransactionGen works when mounted on a UniConf subtree.
WVTEST_MAIN("UniTransactionGen submount test")
{
    UniConfRoot root("temp:");
    UniConf subtree = root["subtree"];

    UniConfRoot transaction(new UniTransactionGen(new UniUnwrapGen(subtree)));

    transaction.remove();
    transaction.xset("key", "value");
    WVPASSEQ(transaction["key"].getme("default"), WvString("value"));
    WVPASSEQ(subtree["key"].getme("default"), WvString("default"));

    transaction.commit();

    WVPASSEQ(transaction["key"].getme("default"), WvString("value"));
    WVPASSEQ(subtree["key"].getme("default"), WvString("value"));
}


static WvString last_key;

static void incr_callback(int *i,
			  const UniConf &handle, const UniConfKey &key)
{
    (*i)++;
    last_key = handle[key].fullkey();
    wvout->print("callback '%s' = '%s'\n", 
	   handle[key].fullkey(), handle[key].getme());
}


WVTEST_MAIN("excessive callbacks")
{
    int i1 = 0, i2 = 0, i3 = 0;
    UniConfRoot uni("temp:");
    UniWatch w1(uni, wv::bind(&incr_callback, &i1, _1, _2), true);
    UniWatch w2(uni["a/b"], wv::bind(&incr_callback, &i2, _1, _2), true);
    
    uni.xsetint("/a/b/c/1", 11); // a, b, c, 1
    uni.xsetint("/a/b/c/2", 22); // 2
    uni.xsetint("/a/e/c/3", 33); // e, c, 3
    uni.xsetint("/a/e/c/4", 44); // 4
    
    WVPASSEQ(i1, 9);
    WVPASSEQ(i2, 4);
    WVPASSEQ(last_key, "a/e/c/4");
    
    UniConfRoot temp("temp:");
    UniConf t(temp["t"]);
    UniWatch w3(temp, wv::bind(&incr_callback, &i3, _1, _2), true);
    WVPASSEQ(i3, 0);
    t.mountgen(new UniTransactionGen(new UniUnwrapGen(uni["a"])), true);
    t.mountgen(new UniTransactionGen(new UniUnwrapGen(uni["a"])), true);
    i3 = 0; // FIXME uncertain what value should be at this point
    WVPASS(t.exists());
    WVPASS(t["b"].exists());
    WVPASSEQ(i3, 0);
    
    // start with fresh counters
    i1 = i2 = i3 = 0;
    
    temp.refresh();
    WVPASSEQ(i3, 0);
    temp.commit();
    WVPASSEQ(i3, 0);
    i3 = 0;
    
    uni.xsetint("/a/b/c/1", 111);
    WVPASSEQ(i1, 1);
    WVPASSEQ(i2, 1);
    WVPASSEQ(i3, 2); // both unwrapgens notify us

    i1 = i2 = i3 = 0;
    t.xsetint("/b/c/2", 222);
    WVPASSEQ(i1, 0);
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 1); // only one transactiongen sees the change
    
    i1 = i2 = i3 = 0;
    t.commit();
    WVPASSEQ(i1, 1);
    WVPASSEQ(i2, 1);
    WVPASSEQ(i3, 1); // opposite transactiongen now sees the change
    WVPASSEQ(last_key, "t/b/c/2");
    WVPASSEQ(uni.xgetint("/a/b/c/2", 0), 222);
    
    i1 = i2 = i3 = 0;
    t.refresh();
    WVPASSEQ(i1, 0);
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 0);
    
    t.remove();
    i1 = i2 = i3 = 0;
    t.xsetint("b/c/1", 111); // /, b, c, 1
    t.xsetint("b/c/2", 222);  // 2
    t.xsetint("e/c/3", 33);  // e, c, 3
    t.xsetint("e/c/4", 44);  // 4
    WVPASSEQ(i1, 0);
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 9);
    
    i1 = i2 = i3 = 0;
    t.commit();
    WVPASSEQ(i1, 0); // no actual changes made here
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 0);
    
    t.remove();
    i1 = i2 = i3 = 0;
    t.refresh();
    WVPASSEQ(i1, 0);
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 9);
    
    i1 = i2 = i3 = 0;
    uni.refresh();
    WVPASSEQ(i1, 0);
    WVPASSEQ(i2, 0);
    WVPASSEQ(i3, 0);
}

static int ncount = 0;
class NCounter {
public:
    void callback(const UniConf keyconf, const UniConfKey _key)
    {
	ncount++;
	wvcon->print("got callback for '%s' '%s'\n",
		keyconf[_key].fullkey(), keyconf[_key].getme());
    }
};

WVTEST_MAIN("double notifications with daemon")
{
    UniConfRoot uniconf;

    signal(SIGPIPE, SIG_IGN);

    WvString sockname("/tmp/unitransgen-%s", getpid());

    UniConfTestDaemon daemon(sockname, "temp:", 
            UniConfTestDaemon::autoinc_server_cb);

    int num_tries = 0;
    const int max_tries = 20;
    while (!uniconf.isok() && num_tries < max_tries)
    {
        num_tries++;
        WVFAIL(uniconf.isok());

        // Try again...
        uniconf.unmount(uniconf.whichmount(), true);
        uniconf.mount(WvString("unix:%s", sockname));
        sleep(1);
    }
    WVPASS(uniconf.isok());

    UniWatchList watches;
    NCounter *foo = new NCounter;
    UniConfCallback uc(wv::bind(&NCounter::callback, foo, _1, _2));
    watches.add(uniconf["Users"], uc);

    uniconf["users"]["x"].setme("1");
    uniconf.commit();

    ncount = 0;
    uniconf["users"]["y"].setme("1");
    uniconf.commit();

    printf("have ncount = %d\n", ncount);

    // FIXME This is the problem
    WVPASSEQ(ncount, 1);
    
    delete foo;
}

WVTEST_MAIN("transaction wrapper")
{
    UniConfRoot uni("temp:");

    UniTransaction trans(uni);

    uni.xset("a/b/c", "foo");
    uni.xset("a/c/d", "bar");
    uni.xset("b/b", "baz");
    WVPASSEQ(uni.xget("a/b/c"), "foo");
    WVPASSEQ(uni.xget("a/c/d"), "bar");
    WVPASSEQ(uni.xget("b/b"), "baz");
    WVPASSEQ(trans.xget("a/b/c"), "foo");
    WVPASSEQ(trans.xget("a/c/d"), "bar");
    WVPASSEQ(trans.xget("b/b"), "baz");

    trans.xset("a/b/c", "baz");
    trans.xset("b/b", "foo");
    WVPASSEQ(uni.xget("a/b/c"), "foo");
    WVPASSEQ(uni.xget("b/b"), "baz");
    WVPASSEQ(trans.xget("a/b/c"), "baz");
    WVPASSEQ(trans.xget("b/b"), "foo");

    trans.refresh();
    WVPASSEQ(trans.xget("a/b/c"), "foo");
    WVPASSEQ(trans.xget("b/b"), "baz");

    trans.xset("a/b/c", "baz");
    trans.xset("b/b", "foo");
    trans.commit();
    WVPASSEQ(uni.xget("a/b/c"), "baz");
    WVPASSEQ(uni.xget("b/b"), "foo");
    WVPASSEQ(trans.xget("a/b/c"), "baz");
    WVPASSEQ(trans.xget("b/b"), "foo");
}

WVTEST_MAIN("bachelor generator")
{
    UniConfRoot a("transaction:temp:");
    UniTransaction b(a);

    a.xset("a/b", "foo");
    WVPASSEQ(a.xget("a/b"), "foo");
    WVPASSEQ(b.xget("a/b"), "foo");
    a.refresh();
    WVPASSEQ(a.xget("a/b"), WvString::null);
    WVPASSEQ(b.xget("a/b"), WvString::null);

    a.xset("a/b", "foo");
    WVPASSEQ(a.xget("a/b"), "foo");
    WVPASSEQ(b.xget("a/b"), "foo");
    a.commit();
    WVPASSEQ(a.xget("a/b"), "foo");
    WVPASSEQ(b.xget("a/b"), "foo");

    a.xset("a/b", "bar");
    WVPASSEQ(a.xget("a/b"), "bar");
    WVPASSEQ(b.xget("a/b"), "bar");

    b.xset("a/b", "baz");
    WVPASSEQ(a.xget("a/b"), "bar");
    WVPASSEQ(b.xget("a/b"), "baz");
    b.refresh();
    WVPASSEQ(a.xget("a/b"), "bar");
    WVPASSEQ(b.xget("a/b"), "bar");

    b.xset("a/b", "baz");
    WVPASSEQ(a.xget("a/b"), "bar");
    WVPASSEQ(b.xget("a/b"), "baz");
    b.commit();
    WVPASSEQ(a.xget("a/b"), "baz");
    WVPASSEQ(b.xget("a/b"), "baz");
}


// this reproduces a really convoluted crash caused by nested generators
// when commit() on the outer generator creates an iterator on a subtree
// of the inner generator, and then that subtree gets deleted while the
// iterator still exists.  Nowadays this crash shouldn't be around anymore,
// because UniTransactionGen returns only "safe" iterators that can handle
// changes to the underlying data structure during their existence.
WVTEST_MAIN("nested transaction commit/replace")
{
    UniConfRoot root("temp:");
    UniTransaction t1(root);
    UniTransaction t2(t1);
    
    t1[5].remove();
    t1[5].xset(6, "hello");
    
    t2[5].remove();
    t2[5].xset(6, "yank");
    t2[5][6].remove();
    WVPASSEQ(t2[5].xget(6), NULL);
    WVPASSEQ(t1[5].xget(6), "hello");
    
    WVPASS("committing");
    t2.commit();
    WVPASS("commit done");
    
    WVPASSEQ(t1[6].xget(6), NULL);
    
    WVPASS("didn't crash");
}


#if 1 // BUGZID: 13167
static int callback_count;

static void callback(const UniConf keyconf, const UniConfKey key)
{
    wvout->print("Handling callback with fullkey '%s', value '%s'\n",
	   keyconf[key].fullkey(),
	   keyconf[key].getme());
    ++callback_count;
}


WVTEST_MAIN("transaction and list interaction")
{
    ::unlink("tmp.ini");
    WvFile file("tmp.ini", O_WRONLY|O_TRUNC|O_CREAT);
    file.write("[a]\n"
	       "b = c\n");
    WVPASS(file.isok());

    UniTempGen *ini = new UniTempGen();
    UniTempGen *def = new UniTempGen();
    UniConfGenList *l = new UniConfGenList();
    l->append(ini, false);
    l->append(def, false);
    UniListGen *cfg = new UniListGen(l);

    UniConfRoot uniconf;
    uniconf["ini"].mountgen(ini);
    uniconf["default"].mountgen(def);
    uniconf["cfg"].mountgen(cfg);

    UniTransaction uni(uniconf);

    callback_count = 0;
    UniWatchList watches;
    watches.add(uni["ini/a/b"], &callback);
    watches.add(uni["cfg/a/b"], &callback);

    (void)UniConfRoot("ini:tmp.ini").copy(uni["/ini"], true);
    uni.commit();

    WVPASSEQ(uni.xget("/ini/a/b"), "c");
    WVPASSEQ(uni.xget("/cfg/a/b"), "c");
    WVPASSEQ(callback_count, 2);
}
#endif // BUGZID: 13167


#if 1 // BUGZID: 14057
template <int place>
int digit(int num)
{
    for (int i = place; i > 1; --i)
	num /= 10;
    return num % 10;
}

template <>
int digit<1>(int num)
{
    return num % 10;
}

template <>
int digit<2>(int num)
{
    return num / 10 % 10;
}

template <>
int digit<3>(int num)
{
    return num / 100 % 10;
}

template <>
int digit<4>(int num)
{
    return num / 1000 % 10;
}

template <>
int digit<5>(int num)
{
    return num / 10000 % 10;
}

static void cb(const UniConf &conf, const UniConfKey &key)
{
}

WVTEST_MAIN("processing many keys")
{
    //signal(SIGALRM, SIG_IGN);
//     ::alarm(1000);
    signal(SIGPIPE, SIG_IGN);

    WvString sockname("/tmp/unitransgen-%s", getpid());

    UniConfTestDaemon daemon(sockname, "temp:", 
            UniConfTestDaemon::autoinc_server_cb);

    UniConfRoot cfg(WvString("transaction:unix:%s", sockname));

    printf("Add Callback\n");
    fflush(stdout);
    cfg.add_callback(NULL, "/", UniConfCallback(cb));

    printf("Setting\n");
    fflush(stdout);
    for (int i = 0; i < 200; ++i)
    {
        int a = digit<5>(i);
        int b = digit<4>(i);
        int c = digit<3>(i);
        int d = digit<2>(i);
        int e = digit<1>(i);
        cfg[a][b][c][d][e].setmeint(i);
    }

    printf("Committing\n");
    fflush(stdout);
    cfg.commit();

    printf("Del Callback\n");
    fflush(stdout);
    cfg.del_callback(NULL, "/");
}
#endif // BUGZID: 14057
