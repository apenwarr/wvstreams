#include "unitransactiongen.h"
#include "uniconf.h"
#include "unitempgen.h"
#include "uniunwrapgen.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "wvhashtable.h"
#include "wvtest.h"

static WvMap<UniConfKey, WvString> callbacks2(5);
static WvMap<UniConfKey, WvString> callbacks1(5);

static void check_callback2(const UniConf &handle,
			    const UniConfKey &key)
{
    fprintf(stderr, "Callback on key \"%s\" with value \"%s\".\n",
	   key.cstr(), handle[key].getme().cstr());
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
    fprintf(stderr, "Callback on key \"%s\" with value \"%s\".\n",
	   key.cstr(), handle[key].getme().cstr());
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
    UniConfRoot two((UniConfGen*)
		    wvcreate<IUniConfGen>("transaction",
					  new UniUnwrapGen(one)));
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

    // Test that it changes back to the empty string. Also, in r4_2
    // and below, test that deleting keys in the underlying generator
    // to which we are making changes other than a tree replacement
    // fails to cause the necessary callbacks for children of that
    // key.

    callbacks2.add("cfg", "");
    callbacks2.add("cfg/Global/Have Disk/Really Have Disk", WvString::null);
    one.xset("cfg", WvString::null);
    WVPASS(!callbacks2.exists("cfg"));
    WVFAIL(!callbacks2.exists("cfg/Global/Have Disk/Really Have Disk"));
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
    printf("callback %p '%s' = '%s'\n", 
	   i, last_key.cstr(), handle[key].getme().cstr());
}


WVTEST_MAIN("excessive callbacks")
{
    int i1 = 0, i2 = 0, i3 = 0;
    UniConfRoot uni("temp:");
    UniWatch w1(uni,
		WvBoundCallback<UniConfCallback,int *>(&incr_callback, &i1),
		true);
    UniWatch w2(uni["a/b"],
		WvBoundCallback<UniConfCallback,int *>(&incr_callback, &i2),
		true);
    
    uni.xsetint("/a/b/c/1", 11); // a, b, c, 1
    uni.xsetint("/a/b/c/2", 22); // 2
    uni.xsetint("/a/e/c/3", 33); // e, c, 3
    uni.xsetint("/a/e/c/4", 44); // 4
    
    WVPASSEQ(i1, 9);
    WVPASSEQ(i2, 4);
    WVPASSEQ(last_key, "a/e/c/4");
    
    UniConfRoot temp("temp:");
    UniConf t(temp["t"]);
    UniWatch w3(temp,
		WvBoundCallback<UniConfCallback,int *>(&incr_callback, &i3),
		true);
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
