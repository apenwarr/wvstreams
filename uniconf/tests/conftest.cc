/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "wvlog.h"
#include "wvlogrcv.h"
#include "wvdiriter.h"
#include "wvfile.h"
#include "wvtclstring.h"
#include "uniconf.h"
#include "strutils.h"
#include "wvconf.h"
#include "uniconftree.h"
#include "uniconfgen.h"

#include <sys/stat.h>

static void copyall(UniConf &src, UniConf &dest)
{
    UniConf::RecursiveIter it(src);
    for (it.rewind(); it.next(); )
    {
        if (it->value().isnull())
            continue;
        //wverr->print("set: \"%s\" = \"%s\"\n", it->fullkey(), it->value());
        dest.set(it->fullkey(), it->value());
        //wverr->print("get: \"%s\"\n", dest.get(it->fullkey()));
    }
}


#if 0
class HelloGen : public UniConfGen
{
public:
    WvString defstr;
    int count;
    
    HelloGen(WvStringParm _def = "Hello World") :
        UniConfGen(WvString("hello://%s", _def)),
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


class UniConfFileTreeGen : public UniConfGen
{
public:
    WvString basedir;
    WvLog log;
    UniConfValueTree root;
    
    UniConfFileTreeGen(WvStringParm _basedir);
    virtual ~UniConfFileTreeGen() { }

    /***** Overridden members *****/

    virtual UniConfLocation location() const;
    virtual bool refresh(const UniConfKey &key, UniConf::Depth depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm)
        { return false; }
    virtual bool zap(const UniConfKey &key)
        { return false; }

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
    NodeIter(const NodeIter &other) : xit(other.xit)
        { }
    virtual ~NodeIter()
        { }

    /***** Overridden methods *****/

    virtual NodeIter *clone() const
        { return new NodeIter(*this); }
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
    refresh(UniConfKey::EMPTY, UniConf::INFINITE);
}


UniConfLocation UniConfFileTreeGen::location() const
{
    return WvString("filetree://%s", basedir);
}


bool UniConfFileTreeGen::refresh(const UniConfKey &key,
    UniConf::Depth depth)
{
    WvString filename("%s%s", basedir, key);
    
    if (depth == UniConf::ZERO || depth == UniConf::ONE ||
        depth == UniConf::INFINITE)
    {
        struct stat statbuf;
        bool exists = lstat(filename.cstr(), & statbuf) == 0;
        
        UniConfValueTree *node = root.find(key);
        if (! exists)
        {
            if (node != & root)
                delete node;
            return true;
        }
        node = maketree(key);
        node->zap();
    }
    
    bool recurse = false;
    switch (depth)
    {
        case UniConf::ZERO:
            return true;
        case UniConf::ONE:
        case UniConf::CHILDREN:
            break;
        case UniConf::INFINITE:
        case UniConf::DESCENDENTS:
            recurse = true;
            break;
    }

    WvDirIter dirit(filename, recurse);
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
    if (node && ! node->value().isnull())
        return node->value();

    // read the file and extract the first non-black line
    WvString filename("%s%s", basedir, key);
    WvFile file(filename, O_RDONLY);
    
    char *line;
    for (;;)
    {
        line = NULL;
        if (! file.isok())
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

    if (! node)
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
    if (! node)
    {
        refresh(key, UniConf::ZERO);
        node = root.find(key);
    }
    return node != NULL;
}


bool UniConfFileTreeGen::haschildren(const UniConfKey &key)
{
    UniConfValueTree *node = root.find(key);
    if (! node || ! node->haschildren())
    {
        refresh(key, UniConf::CHILDREN);
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
    UniConfValueTree *node = & root;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        UniConfValueTree *prev = node;
        node = node->findchild(it());
        if (! node)
            node = new UniConfValueTree(prev, it(), WvString::null);
    }
    return node;
}



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
	
	UniConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
	UniConfKey key2(key);
        UniConfKey key3(key.removefirst(5));
        UniConfKey key4(key.removefirst(900));
	log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Basic config test begins\n");
	
	UniConf cfg;
        cfg.mount(UniConfLocation("temp://"));
	cfg.set("/foo/blah/weasels", "chickens");
	
	cfg["foo"]["pah"]["meatballs"] = 6;
	
	UniConf &x = cfg["snort/fish/munchkins"];
	x.set("big/bad/weasels", 7);
	x["foo"] = x["blue"] = x["true"] = "sneeze";
	
	log("Config dump:\n");
	cfg.dump(quiet);
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
	    h ? h->value() : WvString("NONE"));
	
	// should be defbob comment
	h = cfg["/users/bob/comment"].find_default();
	log("Default for bob: '%s'\n",
            h ? h->value() : WvString("NONE"));
	
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
    
    {
	wvcon->print("\n\n");
	log("-- FileTree test begins\n");
	
	UniConf cfg;
        cfg.mountgen(new UniConfFileTreeGen("/etc/modutils"));
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test begins\n");
	
	UniConf cfg;
	UniConf *cfg2 = &cfg["/weaver ini test"];
	
        cfg.mount(UniConfLocation("readonly://ini://test.ini"));
        cfg2->mount(UniConfLocation("readonly://ini://tmp/weaver.ini"));
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test2 begins\n");
	
	UniConf cfg;
	UniConf &h1 = cfg["/1"], &h2 = cfg["/"];
	
	h1.mount(UniConfLocation("ini://test.ini"));
        h2.mount(UniConfLocation("ini://test2.ini"));
        cfg.refresh();

        UniConf newcfg;
        UniConf &newh1 = newcfg["/1"], &newh2 = newcfg["/"];
        
	newh1.mount(UniConfLocation("ini://test.ini.new"));
        newh2.mount(UniConfLocation("ini://test2.ini.new"));
        newcfg.zap();
	
	log("Partial config dump (branch 1 only):\n");
	h1.dump(quiet);
        
	log("Trying to save unchanged branches:\n");
	cfg.commit();

        copyall(cfg, newcfg);
        newh1.dump(quiet);

        log("Trying to save copy:\n");
	newcfg.commit();
	
	log("Changing some data:\n");
	if (! newh1.exists("big/fat/bob"))
	    newh1.setint("big/fat/bob", 0);
	newh1["big/fat/bob"] = newh1.getint("big/fat/bob") + 1;
	newh1["chicken/hammer\ndesign"] = "simple test";
	newh1["chicken/whammer/designer\\/code\nweasel"] = "this\n\tis a test  ";

        log("Full config dump:\n");
	newcfg.dump(quiet);
	
	log("Saving changed copy:\n");
	newcfg.commit();
    }
    
    return 0;
}
