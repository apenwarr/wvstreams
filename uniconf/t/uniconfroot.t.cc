#include "wvtest.h"
#include "uniconfroot.h"

WVTEST_MAIN("no generator")
{
    UniConfRoot root1;
    WVPASS(root1.getme() == WvString::null);

    // verify that setting keys in unmounted space does not actually
    // set their values (or more likely, cause a segfault)
    UniConfRoot root2;
    root2["subt/mayo/baz"].mount("temp:");
    root2["/"].setme("foo");
    root2["subt"].setme("bar");
    root2["subt/mayo"].setme("moo");
    WVPASS(strcmp(root2["/"].getme().cstr(), "") == 0);
    WVPASS(strcmp(root2["subt"].getme().cstr(), "") == 0);
    WVPASS(strcmp(root2["subt/mayo"].getme().cstr(), "") == 0);        
    WVFAIL(root2["dialer 2"].exists());

}


WVTEST_MAIN("null generator")
{
    UniConfRoot root("null:");
}


// if this test compiles at all, we win!
WVTEST_MAIN("type conversions")
{
    UniConfRoot root("temp:");
    UniConf cfg(root);
    
    cfg.setme("x");
    cfg.setme(5);
    WVPASSEQ("5", cfg.getme("x"));
    WVPASSEQ("5", cfg.getme(5));
    
    cfg["x"].setme("x");
    cfg[5].setme("x");
    int x = 5;
    WVPASSEQ("x", cfg[x].getme("x"));
    
    cfg[cfg.getme(5)].setme("x");
    cfg.setme(cfg.getme(x));
    cfg.setme(cfg.getmeint(x));
    cfg.setmeint(cfg.getmeint(x));
    cfg.setmeint(cfg.getme(x).num());
    
    cfg[WvString(x)].setme("x");
    cfg[WvFastString(x)].setme("x");
    cfg[WvString(5)].setme("x");
    
    cfg.setme(7);
    cfg[6].setme(*cfg);
    cfg.setme(cfg->num());
    WVPASSEQ("7", *cfg);
    
    cfg.xset("sub", "y");
    WVPASSEQ("y", *cfg["sub"]);
    WVPASSEQ("y", cfg.xget("sub", "foo"));
    WVPASSEQ(55, cfg.xgetint("sub", 55)); // unrecognizable string
    WVPASSEQ(7, cfg.xgetint(6));
    
    cfg.xsetint("sub", 99);
    WVPASSEQ(99, cfg["sub"].getmeint(55));
    
    WVPASSEQ("zz", cfg["blah"]->ifnull("zz"));
}

WVTEST_MAIN("case")
{
    UniConfRoot root("temp:");
    UniConf cfg(root);

    cfg.xset("eth0/IPAddr", "10");
    WVPASSEQ("10", cfg.xget("eth0/IPAddr"));
    WVPASSEQ("10", cfg.xget("eth0/ipaddr"));
}

WVTEST_MAIN("haschildren() and exists()")
{
    UniConfRoot root;
    {
    UniConf cfg(root);
    
    WVFAIL(cfg["/"].haschildren());
    WVFAIL(cfg["/"].exists());
    
    cfg.mount("temp:");
    
    WVFAIL(cfg["/"].haschildren());
    WVPASS(cfg["/"].exists());
    }
   
    {
    UniConf cfg(root);
    
    WVFAIL(cfg["/bar"].haschildren());
    WVFAIL(cfg["/bar"].exists());
    
    cfg["/bar/config"].mount("temp:");
    
    WVFAIL(cfg["/bar/config"].haschildren());
    WVPASS(cfg["/bar/config"].exists());
    
    //once something is mounted, parent keys should exist
    WVPASS(cfg["/"].haschildren());
    WVPASS(cfg["/"].exists());
    WVPASS(cfg["/bar"].haschildren());
    WVPASS(cfg["/bar"].exists());
    
    cfg.mount("temp:");
    cfg.xset("/config/bar/foo", "goo");
    WVPASS(cfg["/"].haschildren());
    WVPASS(cfg["/"].exists());
    WVFAIL(cfg["/foo"].exists());
    
    cfg.xset("/foo", "bar");
    
    WVPASS(cfg["/foo"].exists());
    }
}


WVTEST_MAIN("section collapsing")
{
    UniConfRoot r;
    WVPASSEQ(r[""][""]["/"]["simon"][""][""].fullkey().printable(), "simon/");
    WVPASSEQ(r[""][""]["/"]["simon"][""]["/"].fullkey().printable(), "simon/");
}


/* Commented out until fullkey is fixed
WVTEST_MAIN("fullkey()")
{
    UniConfRoot root;
    root.mount("temp:");
    UniConf cfg(root["bleep"]);
    cfg["/foo/bar/blah"].setme("mink");
    WVPASSEQ(cfg["mink"].fullkey(cfg).cstr(), "/foo/bar/blah/mink");
}*/

static int itcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::Iter i(cfg);
    for (i.rewind(); i.next(); )
	count++;
    return count;
}


static int ritcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::RecursiveIter i(cfg);
    for (i.rewind(); i.next(); )
    {
	fprintf(stderr, "key: '%s'\n", i->fullkey(cfg).cstr());
	count++;
    }
    return count;
}


WVTEST_MAIN("iterators")
{
    UniConfRoot root("temp:");
    root["1"].setme("foo");
    root["2"].setme("blah");
    root["2/2b"].setme("sub");
    root["x/y/z/a/b/1/2/3"].setme("something");
    
    WVPASSEQ(itcount(root), 3);
    WVPASSEQ(itcount(root["2"]), 1);
    WVPASSEQ(itcount(root["1"]), 0);
    WVPASSEQ(itcount(root["2/2b"]), 0);
    WVPASSEQ(itcount(root["3"]), 0);

    WVPASSEQ(ritcount(root), 11);
    WVPASSEQ(ritcount(root["2"]), 1);
    WVPASSEQ(ritcount(root["1"]), 0);
    WVPASSEQ(ritcount(root["2/2b"]), 0);
    WVPASSEQ(ritcount(root["3"]), 0);
    WVPASSEQ(ritcount(root["x/y/z/a/b"]), 3);
    
    UniConf sub(root["x/y/z/a"]);
    UniConf::RecursiveIter i(sub);
    i.rewind(); i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b/1");

}

// bug 6869
static int compare(const UniConf &_a, const UniConf &_b)
{
    return strcmp(_a.key().cstr(), _b.key().cstr());
}
WVTEST_MAIN("sorted iterators")
{
    UniConfRoot root("temp:");
    root["3"].setme("foo1");
    root["2"].setme("foo2");
    root["1"].setme("foo3");
    root["4"].setme("foo4");

    UniConf sub(root);
    UniConf::SortedIter i(sub, &compare);
    i.rewind();
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "1");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "2");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "3");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "4");
}

WVTEST_MAIN("nested iterators")
{
    UniConfRoot root("temp:");
    UniConf cfg(root);
    UniConf::Iter i1(cfg);
    cfg["foo"].setmeint(1);
    cfg["/foo/bar"].setmeint(1);
    cfg["/foo/car/bar"].setmeint(1);
    WVPASS(cfg["foo"].getmeint());
    
    for (i1.rewind(); i1.next();)
    {
        UniConf::RecursiveIter i2(cfg);
        for (i2.rewind(); i2.next();)
        {
            i2->getme();
        }
    }
    WVPASS(cfg["foo"].getmeint());
    WVPASS(cfg["/foo/bar"].getmeint());
}


WVTEST_MAIN("mounting with paths prefixed by /")
{
    UniConfRoot root("temp:");
    UniConf cfg1(root["/config"]);
    UniConf cfg2(root["config"]);
    int i = 0;
    
    for (i = 0; i < 5; i++)
        root.xsetint(WvString("/config/bloing%s", i), 1);
    
    for (i = 0; i < 5; i++)
    {
        WVPASS(cfg1.xgetint(WvString("/bloing%s", i)));
        WVPASS(cfg2.xgetint(WvString("/bloing%s", i)));
    }
                
    UniConf::Iter iter1(cfg1); 
    UniConf::Iter iter2(cfg1); 
    i = 0;
    for (iter1.rewind(); iter1.next(); i++)
        WVPASS(iter1->getmeint());
    
    i = 0;
    for (iter2.rewind(); iter2.next(); i++)
        WVPASS(iter2->getmeint());

}

WVTEST_MAIN("Deleting while iterating")
{
    UniConfRoot root("temp:");
    char *foo = new char[250]; //to make sure the hash moves in memory
    for (int i = 0; i < 10; i++)
    {
        root.xsetint(i, i);
        root[i].xsetint(i, i);
    }
    
    UniConf::Iter i(root);
    char *foo2 = new char[250];
    for (i.rewind(); i.next(); )
    {
//        fprintf(stderr, "%s\n", i->getme().cstr());
        root[i->key()].setme(WvString::null);
        if (i->getme() != WvString::null)
            i.rewind();
    }
    deletev foo;
    deletev foo2;
}

void verify_recursive_iter(UniConfRoot &root)
{
    // verify that recursive iteration works as expected
    UniConf::RecursiveIter i(root);
    i.rewind(); 
    WVPASS(i.next());
    WVPASS(strcmp(i().key().cstr(), "subt") == 0);
    WVPASS(strcmp(i().getme().cstr(), "") == 0);
    WVPASS(i.next());
    WVPASS(strcmp(i().key().cstr(), "mayo") == 0);
    WVPASS(strcmp(i().getme().cstr(), "baz") == 0);
    WVFAIL(i.next());

    // verify that non-recursive iteration doesn't go over keys
    // it's not supposed to
    UniConf::Iter j(root);
    j.rewind(); 
    WVPASS(j.next());
    WVPASS(strcmp(j().key().cstr(), "subt") == 0);
    WVPASS(strcmp(j().getme().cstr(), "") == 0);
    WVFAIL(j.next());
}

WVTEST_MAIN("Recursive iteration with no generator at root")
{       
    // distance of "2" between first mount and unmounted root
    UniConfRoot root1;
    root1["subt/mayo"].mount("temp:");
    root1["subt/mayo"].setme("baz");        
    verify_recursive_iter(root1);

    // distance of "1" between first mount and unmounted root
    UniConfRoot root2;
    root2["subt"].mount("temp:");
    root2["subt/mayo"].setme("baz");
    verify_recursive_iter(root2);
}
