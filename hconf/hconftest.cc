/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "wvhconf.h"
#include "wvlog.h"
#include "wvconf.h"
#include "wvdiriter.h"
#include "wvfile.h"


class HelloGen : public WvHConfGen
{
public:
    WvString defstr;
    int count;
    
    HelloGen(WvStringParm _def = "Hello World")
	: defstr(_def) { count = 0; }
    virtual void update(WvHConf *h);
};


void HelloGen::update(WvHConf *h)
{
    wvcon->print("Hello: updating %s\n", h->full_key());
    *h = WvString("%s #%s", defstr, ++count);
    h->dirty = false;
}


class WvHConfFileTree : public WvHConfGen
{
public:
    WvString basedir;
    WvHConf *top;
    WvLog log;
    
    WvHConfFileTree(WvHConf *_top, WvStringParm _basedir);
    virtual void update(WvHConf *h);
    virtual void load();
};


WvHConfFileTree::WvHConfFileTree(WvHConf *_top, WvStringParm _basedir)
    : basedir(_basedir), log("FileTree", WvLog::Info)
{
    top = _top;
    log(WvLog::Notice,
	"Creating a new FileTree based on '%s' at location '%s'.\n",
	basedir, top->full_key());
}


// use the first nonblank line in the file as the config contents.
void WvHConfFileTree::update(WvHConf *h)
{
    char *line;
    WvString name("/%s", h->gen_full_key());
    WvFile f(name, O_RDONLY);
    
    if (!f.isok())
	log(WvLog::Warning, "Error reading %s: %s\n", name, f.errstr());
    
    while (f.isok())
    {
	line = f.getline(-1);
	if (!line)
	    continue;
	line = trim_string(line);
	if (!line[0])
	    continue;
	
	*h = line;
	h->dirty = false;
	return;
    }
}


void WvHConfFileTree::load()
{
    WvHConf *h;
    WvDirIter i(basedir, true);
    
    for (i.rewind(); i.next(); )
    {
	log(WvLog::Debug2, ".");
	h = make_tree(top, i->fullname);
    }
}


class WvHConfIniFile : public WvHConfGen
{
public:
    WvString filename;
    WvHConf *top;
    WvLog log;
    
    WvHConfIniFile(WvHConf *_top, WvStringParm _filename);
    virtual void load();
    virtual void save();
    
    void save_subtree(WvStream &out, WvHConf *h, WvHConfKey key);
};


WvHConfIniFile::WvHConfIniFile(WvHConf *_top, WvStringParm _filename)
    : filename(_filename), log(filename)
{
    top = _top;
    log(WvLog::Notice, "Using IniFile '%s' at location '%s'.\n", 
	filename, top->full_key());
}


void WvHConfIniFile::load()
{
    char *line, *cptr;
    WvHConf *h;
    WvFile f(filename, O_RDONLY);
    WvString section = "";
    
    if (!f.isok())
    {
	log("Can't open config file: %s\n", f.errstr());
	return;
    }
    
    while ((line = f.getline(-1)) != NULL)
    {
	line = trim_string(line);
	
	// beginning of a new section?
	if (line[0] == '[' && line[strlen(line)-1] == ']')
	{
	    line[strlen(line)-1] = 0;
	    section = line+1;
	    section.unique();
	    continue;
	}
	
	// name = value setting?
	cptr = strchr(line, '=');
	if (cptr)
	{
	    *cptr++ = 0;
	    line = trim_string(line);
	    cptr = trim_string(cptr);
	    h = make_tree(top, WvString("%s/%s", section, line));
	    
	    bool d1 = h->dirty, d2 = h->child_dirty;
	    h->set_without_notify(cptr);
	    
	    // loaded _from_ the config file, so that didn't make it dirty!
	    h->dirty = d1;
	    h->child_dirty = d2;
	}
    }
}


void WvHConfIniFile::save()
{
    if (!top->dirty && !top->child_dirty)
	return; // no need to rewrite!
    
    log("Saving %s...", filename);
    
    WvFile out(WvString("%s.new", filename), O_WRONLY|O_CREAT|O_TRUNC);
    save_subtree(out, top, "/");
    out("\n");
}


static bool any_direct_children(WvHConf *h)
{
    if (!h->children)
	return false;
    
    WvHConfDict::Iter i(*h->children);
    for (i.rewind(); i.next(); )
    {
	if (!!*i)
	    return true;
    }
    
    return false;
}

void WvHConfIniFile::save_subtree(WvStream &out, WvHConf *h, WvHConfKey key)
{
    // dump the "root level" of this tree into one section
    if (any_direct_children(h))
    {
	out("\n[%s]\n", key);
	WvHConfDict::Iter i(*h->children);
	for (i.rewind(); i.next(); )
	{
	    if (i->generator && i->generator != this) continue;
	    
	    if (!!*i)
		out("%s = %s\n", i->name, *i);
	}
    }
    
    // dump subtrees into their own sections
    if (h->children)
    {
	WvHConfDict::Iter i(*h->children);
	for (i.rewind(); i.next(); )
	{
	    if (i->generator && i->generator != this)
		i->save();
	    else if (i->children)
	    {
		WvHConfKey key2(key);
		key2.append(&i->name, false);
		save_subtree(out, i.ptr(), key2);
	    }
	}
    }
}


int main()
{
    WvLog log("hconftest", WvLog::Info);
    WvLog quiet("*", WvLog::Debug1);
    
    log("An hconf instance is %s bytes long.\n", sizeof(WvHConf));
    log("A wvconf instance is %s/%s/%s bytes long.\n",
	sizeof(WvConf), sizeof(WvConfigSection), sizeof(WvConfigEntry));
    log("A stringlist is %s bytes long.\n", sizeof(WvStringList));
    
    {
	wvcon->print("\n\n");
	log("-- Key test begins\n");
	
	WvHConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
	WvHConfKey key2(key), key3(key, 5), key4(key, 900);
	log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Basic config test begins\n");
	
	WvHConf cfg;
	cfg.set("/foo/blah/weasels", "chickens");
	
	cfg["foo"]["pah"]["meatballs"] = 6;
	
	WvHConf &x = cfg["snort/fish/munchkins"];
	x.set("big/bad/weasels", 7);
	x["foo"] = x["blue"] = x["true"] = "sneeze";
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Inheritence test begins\n");
	
	WvHConf cfg, *h;
	
	cfg.set("/default/users/*/comment", "defuser comment");
	cfg.set("/default/users/bob/comment", "defbob comment");
	
	log("Old comment settings are: %s/%s\n",
	    cfg["/users/randomperson/comment"], cfg["/users/bob/comment"]);
	
	cfg["/users"].defaults = &cfg["/default/users"];
	
	h = cfg["/users/randomperson/comment"].find_default();
	log("Default for randomperson: '%s'\n", h ? *h : WvString("NONE"));
	
	h = cfg["/users/bob/comment"].find_default();
	log("Default for bob: '%s'\n", h ? *h : WvString("NONE"));
	
	log("New comment settings are: %s/%s\n",
	    cfg["/users/noperson/comment"], cfg["/users/bob/comment"]);
	
	cfg["/users/bob/someone/comment"] = "fork";
	
	log("Config dump 2:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- Hello Generator test begins\n");
	
	WvHConf cfg;
	
	cfg["/hello"].generator = new HelloGen("Hello world!");
	cfg["/bonjour"].generator = new HelloGen("Bonjour tout le monde!");
	
	cfg.get("/bonjour/1");
	cfg.get("/bonjour/2");
	cfg.get("/bonjour/3");
	cfg.get("/hello/3");
	cfg.get("/hello/2");
	cfg.get("/hello/1");
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- FileTree test begins\n");
	
	WvHConf cfg;
	
	cfg.generator = new WvHConfFileTree(&cfg, "/etc/modutils");
	cfg.generator->load();
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test begins\n");
	
	WvHConf cfg;
	WvHConf *cfg2 = &cfg["/weaver ini test"];
	
	cfg.generator = new WvHConfIniFile(&cfg, "test.ini");
	cfg.generator->load();
	
	cfg2->generator = new WvHConfIniFile(cfg2, "/tmp/weaver.ini");
	cfg2->generator->load();
	
	log("Config dump:\n");
	cfg.dump(quiet);
    }
    
    {
	wvcon->print("\n\n");
	log("-- IniFile test2 begins\n");
	
	WvHConf cfg;
	WvHConf &h1 = cfg["/1"], &h2 = cfg["/"];
	
	h1.generator = new WvHConfIniFile(&h1, "test.ini");
	h1.generator->load();
	
	h2.generator = new WvHConfIniFile(&h2, "test2.ini");
	h2.generator->load();
	
	log("Partial config dump (branch 1 only):\n");
	h1.dump(quiet);
	
	log("Trying to save unchanged branches:\n");
	cfg.save();
	
	log("Changing some data:\n");
	if (!h1["big/fat/bob"])
	    h1["big/fat/bob"] = 0;
	h1["big/fat/bob"] = h1["big/fat/bob"].num() + 1;
	h1.dump(quiet);
	
	log("Saving changed data:\n");
	cfg.save();
    }
    
    return 0;
}
