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
    
    HelloGen(const WvString &_def = "Hello World")
	: defstr(_def) { count = 0; }
    virtual void update(WvHConf *h);
};


void HelloGen::update(WvHConf *h)
{
    wvcon->print("Hello: updating %s\n", h->full_key());
    *h = WvString("%s #%s", defstr, ++count);
}


class WvHConfFileTree : public WvHConfGen
{
public:
    WvString basedir;
    WvHConf *top;
    WvLog log;
    
    WvHConfFileTree(WvHConf *_top, const WvString &_basedir);
    virtual void update(WvHConf *h);
    virtual void load();
};


WvHConfFileTree::WvHConfFileTree(WvHConf *_top, const WvString &_basedir)
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
    
    WvHConfIniFile(WvHConf *_top, const WvString &_filename);
    virtual void load();
};


WvHConfIniFile::WvHConfIniFile(WvHConf *_top, const WvString &_filename)
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
	    *h = cptr;
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
    
    return 0;
}
