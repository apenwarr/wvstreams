/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConfEmu.
 */
#include "wvlog.h"
#include "wvlogrcv.h"
#include "wvdiriter.h"
#include "wvconfemu.h"
#include "wvfile.h"
#include "wvtclstring.h"
#include "uniconf.h"
#include "strutils.h"
#include "uniconfroot.h"
#include "uniconftree.h"
#include "uniconfgen.h"

#include <sys/stat.h>

static void copyall(UniConf &src, UniConf &dest)
{
    UniConf::RecursiveIter it(src);
    for (it.rewind(); it.next(); )
    {
        WvString value(it->getme());
        if (value.isnull())
            continue;
        //wverr->print("set: \"%s\" = \"%s\"\n", it->fullkey(), value);
        dest[it->fullkey()].setme(value);
        //wverr->print("get: \"%s\"\n", dest[it->fullkey()].get());
    }
}


#if 0
class HelloGen : public UniConfGen
{
public:
    WvString defstr;
    int count;
    
    HelloGen(WvStringParm _def = "Hello World") :
        UniConfGen(WvString("hello:%s", _def)),
        defstr(_def)
    {
        count = 0;
    }
    virtual void update(UniConf *&h);
};


void HelloGen::update(UniConf *&h)
{
    wvcon->print("Hello: updating %s\n", h->fullkey());
    h->setvalue(WvString("%s #%s", defstr, ++count));
    h->dirty = false;
    h->obsolete = false;
    h->waiting = false;
}
#endif

// FIXME: This class *REALLY* should be merged out.
#if 0
class UniConfFileTreeGen : public UniConfGen
{
public:
    WvString basedir;
    WvLog log;
    UniConfValueTree root;
    
    UniConfFileTreeGen(WvStringParm _basedir);
    virtual ~UniConfFileTreeGen() { }

    /***** Overridden members *****/

    virtual bool refresh();
    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm) { }

    virtual Iter *iterator(const UniConfKey &key);

private:
    UniConfValueTree *maketree(const UniConfKey &key);
    class NodeIter;
};


class UniConfFileTreeGen::NodeIter : public UniConfFileTreeGen::Iter
{
protected:
    UniConfValueTree::Iter xit;

public:
    NodeIter(UniConfValueTree &node) : xit(node)
        { }

    /***** Overridden methods *****/

    virtual void rewind()
        { xit.rewind(); }
    virtual bool next()
        { return xit.next(); }
    virtual UniConfKey key() const
        { return xit->key(); }
};


UniConfFileTreeGen::UniConfFileTreeGen(WvStringParm _basedir) :
    basedir(_basedir), log("FileTree", WvLog::Info),
    root(NULL, UniConfKey::EMPTY, "")
{
    log(WvLog::Notice,
	"Creating a new FileTree based on '%s'.\n", basedir);
}


bool UniConfFileTreeGen::refresh()
{
    WvString filename("%s%s", basedir, key);
    
    struct stat statbuf;
    bool exists = lstat(filename.cstr(), &statbuf) == 0;

    UniConfValueTree *node = root.find(key);
    if (!exists)
    {
        if (node != &root)
            delete node;
        return true;
    }
    node = maketree(key);
    node->zap();
    
    WvDirIter dirit(filename, true);
    UniConfKey dirkey(filename);
    for (dirit.rewind(); dirit.next(); )
    {
	log(WvLog::Debug2, ".");
        UniConfKey filekey(dirit->fullname);
        filekey = filekey.removefirst(dirkey.numsegments());
        maketree(filekey);
    }
    return true;
}


WvString UniConfFileTreeGen::get(const UniConfKey &key)
{
    // check the cache
    UniConfValueTree *node = root.find(key);
    if (node && !node->value().isnull())
        return node->value();

    // read the file and extract the first non-black line
    WvString filename("%s%s", basedir, key);
    WvFile file(filename, O_RDONLY);
    
    char *line;
    for (;;)
    {
        line = NULL;
        if (!file.isok())
            break;
	line = file.getline(-1);
	if (!line)
            break;
	line = trim_string(line);
	if (line[0])
            break;
    }

    if (file.geterr())
    {
	log("Error reading %s: %s\n", filename, file.errstr());
        line = "";
    }

    if (!node)
        node = maketree(key);
    WvString value(line);
    value.unique();
    node->setvalue(value);
    file.close();
    return value;
}


bool UniConfFileTreeGen::exists(const UniConfKey &key)
{
    UniConfValueTree *node = root.find(key);
    if (!node)
    {
        refresh();
        node = root.find(key);
    }
    return node != NULL;
}


bool UniConfFileTreeGen::haschildren(const UniConfKey &key)
{
    UniConfValueTree *node = root.find(key);
    if (!node || !node->haschildren())
    {
        refresh(key, UniConfDepth::CHILDREN);
        node = root.find(key);
    }
    return node != NULL && node->haschildren();
}


UniConfFileTreeGen::Iter *UniConfFileTreeGen::iterator(const UniConfKey &key)
{
    if (haschildren(key))
    {
        UniConfValueTree *node = root.find(key);
        if (node)
            return new NodeIter(*node);
    }
    return new NullIter();
}


UniConfValueTree *UniConfFileTreeGen::maketree(const UniConfKey &key)
{
    // construct a node for the file with a null value
    UniConfValueTree *node = &root;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        UniConfValueTree *prev = node;
        node = node->findchild(it());
        if (!node)
            node = new UniConfValueTree(prev, it(), WvString::null);
    }
    return node;
}
#endif


int main()
{
    WvLog log("conftest", WvLog::Info);
    WvLog quiet("*", WvLog::Debug1);
    // try Debug5 for lots of messages
    WvLogConsole rcv(2, WvLog::Debug4);
    
    log("A wvconf instance is %s/%s/%s bytes long.\n",
	sizeof(WvConf), sizeof(WvConfigSection), sizeof(WvConfigEntry));
    log("A stringlist is %s bytes long.\n", sizeof(WvStringList));
    log("A UniConfValueTree instance is %s bytes long.\n", sizeof(UniConfValueTree));
    log("A UniConfKey instance is %s bytes long.\n", sizeof(UniConfKey));
    
    {
	wvcon->print("\n\n");
	log("-- Key test begins\n");
	
	UniConfKey key("/a/b/c/d/e/f/ghij////k/l/m/");
	UniConfKey key2(key), key2b(key, key);
        UniConfKey key3(key.removefirst(5));
        UniConfKey key4(key.removefirst(900));
        // FIXME: we're just barely scratching the surface here!
	log("key :  %s\n"
	    "key2:  %s (%s, %s)\n"
	    "key2b: %s (%s)\n"
	    "key3:  %s\n"
	    "key4:  %s\n",
	    key, key2, key2.range(1, key2.numsegments()-1), key2.range(0, 0),
	    key2b, key2b.numsegments(),
	    key3, key4);
	
	log("keyiter test 2b: ");
	UniConfKey::Iter i(key2b);
	for (i.rewind(); i.next(); )
	     log("'%s' ", *i);
	log("\n");
    }
    
    {
	wvcon->print("\n\n");
	log("-- Basic config test begins\n");
	
        UniConfRoot cfg("temp:");
	cfg["/foo/blah/weasels"].setme("chickens");
	
	cfg["foo"]["pah"]["meatballs"].setmeint(6);
	
	assert(cfg["/foo/blah"].haschildren());
	assert(!cfg["/foo/blah/weasels"].haschildren());
	
	UniConf x(cfg["snort/fish/munchkins"]);
	x["big/bad/weasels"].setmeint(7);
	x["foo"].setme("sneeze");
        x["blue"].setme("sneeze");
        x["true"].setme("sneeze");
	
	{
	    log("get() test:\n");
	    log("  '%s' '%s' '%s' '%s'/'%s' '%s'\n",
		cfg["foo/blah/weasels"].getme(),
		cfg["foo/blah"].getme(),
		cfg["foo"].getme(),
		cfg[""].getme(), cfg[UniConfKey::EMPTY].getme(),
		cfg.getme()
		);
	}
	
	{
	    log("Toplevel dump:\n");
	    UniConf::Iter i(cfg);
	    for (i.rewind(); i.next(); )
		quiet.print("'%s' = '%s'\n", i->key(), i->getme());
	}
	
	log("Config dump:\n");
	cfg.dump(quiet, true);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Simple deletion test begins\n");
	UniConfRoot cfg("temp:");
	UniConf a(cfg["admins"]), a1(a["1"]), a2(a["2"]);
	a1.setme("11");
	a2.setme("22");
	assert(a.haschildren());
	assert(!a2.haschildren());
	a1.remove();
	assert(a.exists());
	assert(!a1.exists());
	assert(a2.exists());
	assert(a.haschildren());
	assert(!a1.haschildren());
	assert(!a2.haschildren());
    }
    
#if 0
    {
	wvcon->print("\n\n");
	log("-- Inheritence test begins\n");
	
	UniConf cfg, *h;
	
	cfg.set("/default/users/*/comment", "defuser comment");
	cfg.set("/default/users/bob/comment", "defbob comment");
	
	// should be (nil)/(nil)
	log("Old comment settings are: %s/%s\n",
	    cfg.get("/users/randomperson/comment"),
            cfg.get("/users/bob/comment"));
	
	cfg["/users"].defaults = &cfg["/default/users"];

	// should be defuser comment
	h = cfg["/users/randomperson/comment"].find_default();
	log("Default for randomperson(%s): '%s'\n",
            h ? h->fullkey() : "",
	    h ? h->get() : WvString("NONE"));
	
	// should be defbob comment
	h = cfg["/users/bob/comment"].find_default();
	log("Default for bob: '%s'\n",
            h ? h->get() : WvString("NONE"));
	
	// should be defuser comment/defbob comment
	log("New comment settings are: %s/%s\n",
	    cfg.get("/users/noperson/comment"),
            cfg.get("/users/bob/comment"));
	
	cfg.set("/users/bob/someone/comment", "fork");
	
	log("Config dump 2:\n");
	cfg.dump(quiet);
    }
#endif
    
#if 0
    {
	wvcon->print("\n\n");
	log("-- Hello Generator test begins\n");
	
	UniConf cfg;
	
        cfg["/hello"].mount(new HelloGen("Hello world"));
        cfg["/bonjour"].mount(new HelloGen("Bonjour tout le monde!"));
	
	cfg.get("/bonjour/1");
	cfg.get("/bonjour/2");
	cfg.get("/bonjour/3");
	cfg.get("/hello/3");
	cfg.get("/hello/2");
	cfg.get("/hello/1");
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
#endif

#if 0 
    {
	wvcon->print("\n\n");
	log("-- FileTree test begins\n");
	
        UniConfRoot root(new UniConfFileTreeGen("/etc/modutils"));
	UniConf cfg(root);
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
#endif

    {
	wvcon->print("\n\n");
	log("-- IniFile test begins\n");
	
        UniConfRoot root;
	UniConf cfg(root);
	UniConf cfg2(cfg["/weaver ini test"]);
	
        cfg.mount("readonly:ini:test.ini");
        cfg2.mount("readonly:ini:/tmp/weaver.ini");
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
        wvcon->print("\n\n");
        log("-- Defaults test begins\n");

        log("Setting up config file...\n");
        UniConfRoot root("default:ini:uniconf.ini");
        root["*"].setme("go wild, bay-be!");
        root["*/drheld/whee"].setme("3");
        root["users/*/chicken/bork"].setme("b0rk3n g00dn3ss");
        root["users/*/chicken/*"].setme("happy");
        root["users/apenwarr"].setme("ooga booga");
        root["users/apenwarr/chicken/hammer"].setme("smashy!");
        root["users/apenwarr/ftp"].setme("1");
        root["wild/*/*/blink"].setme("*1");
        root["wild/*/*/blank"].setme("*2");
        root["wild/*/*/plunk"].setme("*3");
        root.commit();

        log("Starting tests...\n");

        WvString result = root["stupidthing"].getme();
        log("/stupidthing = %s (should = go wild, bay-be!)\n", result);

        result = root["home/drheld"]["whee"].getme();
        log("/home/drheld/whee = %s (should = 3)\n", result);

        result = root["/users/apenwarr/chicken/bork"].getme();
        log("/users/apenwarr/chicken/bork = %s (should = b0rk3n g00dn3ss)\n",
                                                                    result);

        result = root["/users/silly/chicken/die"].getme();
        log("/users/silly/chicken/die = %s (should = happy)\n", result);

        result = root["/wild/foo/bar/blink"].getme("outtaspace");
        log("/wild/foo/bar/blink = %s (should = bar)\n", result);
        
        result = root["/wild/foo/bar/blank"].getme("outtaspace");
        log("/wild/foo/bar/blank = %s (should = foo)\n", result);
        
        result = root["/wild/foo/bar/plunk"].getme("outtaspace");
        log("/wild/foo/bar/plunk = %s (should = outtaspace)\n", result);
    }

    {
        wvcon->print("\n\n");
        log("-- Generator List test begins\n");
        
        UniConfRoot root("list: readonly:ini:test.ini ini:test2.ini");
        root["chickens"]["boob"].setme("not_frank");

        log("[chickens][bob] = %s\n", root["chickens"]["bob"].getme());
        log("[chickens][boob] = %s\n", root["chickens"]["boob"].getme());

        root["chickens"]["bob"].setme("gooooblefish");
        root["chickens"]["boob"].setme("I like to wear funny hats");
        log("Setting [chickens][bob] = gooooblefish  **(read only on top)\n");
        log("Setting [chickens][boob] = I like to wear funny hats\n");

        log("[chickens][bob] = %s\n", root["chickens"]["bob"].getme());
        log("[chickens][boob] = %s\n", root["chickens"]["boob"].getme());

        root.commit();

    }

    {
	wvcon->print("\n\n");
	log("-- IniFile test2 begins\n");
	
        UniConfRoot root;
	UniConf cfg(root);
	UniConf h1(cfg["/1"]);
        UniConf h2(cfg["/"]);
	
	h1.mount("ini:test.ini", true);
        h2.mount("ini:test2.ini", true);
        
	log("Partial config dump (branch 1 only):\n");
	h1.dump(quiet);
        
	log("Trying to save unchanged branches:\n");
	cfg.commit();

        UniConfRoot newroot;
        UniConf newcfg(newroot);
        UniConf newh1(newcfg["/1"]);
        UniConf newh2(newcfg["/"]);
        
	newh1.mount("ini:test.ini.new", false);
        newh2.mount("ini:test2.ini.new", false);
	
        copyall(cfg, newcfg);
        newh1.dump(quiet);

        log("Trying to save copy:\n");
	newcfg.commit();
	
	log("Changing some data:\n");
	if (!newh1["big/fat/bob"].exists())
	    newh1["big/fat/bob"].setmeint(0);
	newh1["big/fat/bob"].setmeint(newh1["big/fat/bob"].getmeint() + 1);
	newh1["chicken/hammer\ndesign"].setme("simple test");
	newh1["chicken/whammer/designer\\/code\nweasel"].setme(
            "this\n\tis a test  ");

        log("Full config dump:\n");
	newcfg.dump(quiet);
	
	log("Saving changed copy:\n");
	newcfg.commit();
    }
    
    return 0;
}
