#include "wvondiskhash.h"
#include "wvhashtable.h"
#include "wvstream.h"

// a tradition dict to store the key/values we're going to test the bdb hash
// with
struct KeyVal
{
    WvString key;
    WvString val;
    KeyVal(WvStringParm k, WvStringParm v) : key(k), val(v) { }
};
DeclareWvDict(KeyVal, WvString, key);
KeyValDict keyvals(3);
KeyValDict::Iter gi(keyvals);
bool iterdone; 

template <class TestHash> 
class HashTester
{
    typedef typename TestHash::Iter TestHashIter;
public:
    static void go(WvString dbid, WvString bdname)
    {
        unlink(bdname);
        TestHash ss(bdname);
	wvcon->print("\n-------------------------\n");
	wvcon->print("Now testing %s\n", dbid);
	wvcon->print("Populating.... ");
        ss.zap();
        KeyValDict::Iter i(keyvals);
        for (i.rewind(); i.next(); )
            ss.add(i->key, i->val, true);
        wvcon->print("done\n");

        runtests(ss);
    }

    static bool cmptest(WvStringParm wantkey, WvStringParm wantval,
            WvStringParm gotkey, WvStringParm gotval)
    {
        if (wantkey != gotkey || wantval != gotval) {
            wvcon->print("expect '%s'/'%s', got '%s'/'%s' - FAILED\n",
                    wantkey, wantval, gotkey, gotval);
            return false;
        } else {
            wvcon->print("expect '%s'/'%s' - PASSED\n", wantkey, gotval); 
            return true;
        }
    }

    static bool itertest(TestHashIter &i, WvStringParm key,
            WvStringParm want)
    {
        i.rewind(key);
        if (i.next())
            return cmptest(key, want, i.key(), i());
        return true;
    }

    static bool itertest2(TestHash &ss, WvStringParm key, WvStringParm want)
    {
        TestHashIter i(ss);
        return itertest(i, key, want);
    }

    static void itertest3(WvStream &s, void *userdata)
    {
        TestHash &ss = *((TestHash *) userdata);
        iterdone = !gi.next();
        if (!iterdone) itertest2(ss, gi->key, gi->val);
        s.alarm(0);
    }


    static int alpha(const KeyVal *a, const KeyVal *b)
    {
        return strcmp(a->key, b->key);
    }


    static bool runtests(TestHash &ss)
    {
        bool good = true;

        {
            wvcon->print("\nSimple iter:\n");
            TestHashIter i1(ss);
            KeyValDict::Sorter i2(keyvals, alpha);
            i1.rewind(); i2.rewind();
            for (;;)
            {
                bool i1ok = i1.next();
                bool i2ok = i2.next();
                if (i1ok != i2ok)
                    wvcon->print("\nnext() return value out of sync: "
                            "expected %s, got %s - FAILED\n", i2ok, i1ok);
                if (!i1ok || !i2ok) break;

                good = cmptest(i2->key, i2->val, i1.key(), i1()) && good;
            }
        }

        {
            wvcon->print("\nMultiple rewinds:\n");
            TestHashIter i(ss);
            KeyValDict::Iter i2(keyvals);
            for (i2.rewind(); i2.next(); )
                good = itertest(i, i2->key, i2->val) && good;
        }

        {
            wvcon->print("\nRecreation of iter:\n");
            KeyValDict::Iter i2(keyvals);
            for (i2.rewind(); i2.next(); )
                good = itertest2(ss, i2->key, i2->val) && good;
        }

        {
            wvcon->print("\nRecreation of iter through a callback:\n");
            WvStream s;
            s.setcallback(itertest3, &ss);
            s.alarm(0);
            gi.rewind();
            iterdone = false;
            while (!iterdone)
                if (s.select(-1)) s.callback();
        }

        {
            wvcon->print("\nNested iters:\n");
            TestHashIter i1(ss);
            KeyValDict::Sorter i2(keyvals, alpha);
            i2.rewind(); i1.rewind();
            for (;;) 
            {
                bool i1ok = i1.next();
                bool i2ok = i2.next();
                if (i1ok != i2ok)
                    wvcon->print("\nnext() return value out of sync: "
                            "expected %s, got %s - FAILED\n", i2ok, i1ok);
                if (!i1ok || !i2ok) break;

                fprintf(stderr, "\n");
                good = cmptest(i2->key, i2->val, i1.key(), i1()) && good;
                fprintf(stderr, "\n");
                KeyValDict::Iter i3(keyvals);
                for (i3.rewind(); i3.next(); )
                    good = itertest2(ss, i3->key, i3->val) && good;
            }
        }

        {
            wvcon->print("\nUpdate during an iter:\n");
            TestHashIter i1(ss);
            KeyValDict::Sorter i2(keyvals, alpha);
            i2.rewind(); i1.rewind();
            for (;;) 
            {
                bool i1ok = i1.next();
                bool i2ok = i2.next();
                if (i1ok != i2ok)
                    wvcon->print("\nnext() return value out of sync: "
                            "expected %s, got %s - FAILED\n", i2ok, i1ok);
                if (!i1ok || !i2ok) break;

                if (i1.key() == "hello") {
                    i1() = "everybody";
                    i1.save();
                }

                if (i2->key == "hello")
                    good = cmptest(i2->key, "everybody", i1.key(), i1()) && good;
                else
                    good = cmptest(i2->key, i2->val, i1.key(), i1()) && good;
            }
        }

        {
            wvcon->print("\nDelete during an iter:\n");
            TestHashIter i1(ss);
            KeyValDict::Sorter i2(keyvals, alpha);
            i2.rewind(); i1.rewind();
            for (;;) 
            {
                bool i1ok = i1.next();
                bool i2ok = i2.next();
                if (i1ok != i2ok)
                    wvcon->print("\nnext() return value out of sync: "
                            "expected %s, got %s - FAILED\n", i2ok, i1ok);
                if (!i1ok || !i2ok) break;

                if (i1.key() == "hello")
                    i1.xunlink();

                if (i2->key == "hello") {
                    //good = cmptest(i2->key, "everybody", i1.key(), i1()) && good;
                    //key invalid now - how do we test this?
                }
                else
                    good = cmptest(i2->key, i2->val, i1.key(), i1()) && good;
            }
        }

        return good;
    }
};

int main()
{
    keyvals.add(new KeyVal("hello", "world"), true);
    keyvals.add(new KeyVal("i like", "muffins"), true);
    keyvals.add(new KeyVal("/foo/bar", "baz"), true);
    keyvals.add(new KeyVal("/", "slash"), true);
    keyvals.add(new KeyVal("/foo", "kung foo!"), true);
 
    HashTester< WvOnDiskHash<WvString,WvString,WvBdbHash> >
        ::go("Named BDB hash", "test.db");

    HashTester< WvOnDiskHash<WvString,WvString,WvBdbHash> >
        ::go("Anonymous BDB hash", "");

    HashTester< WvOnDiskHash<WvString,WvString,WvQdbmHash> >
        ::go("QDBM hash", "test.db");
}

