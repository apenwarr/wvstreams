/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "wvhconf.h"
#include "wvlog.h"
#include "wvconf.h"


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


int main()
{
    WvLog log("hconftest", WvLog::Info);
    
    log("An hconf instance is %s bytes long.\n", sizeof(WvHConf));
    log("A wvconf instance is %s/%s/%s bytes long.\n",
	sizeof(WvConf), sizeof(WvConfigSection), sizeof(WvConfigEntry));
    log("A stringlist is %s bytes long.\n", sizeof(WvStringList));
    
    {
	log("-- Key test begins\n");
	
	WvHConfKey key("/a/b/c/d/e/f/ghij////k/l/m");
	WvHConfKey key2(key), key3(key, 5), key4(key, 900);
	log("key : %s\nkey2: %s\nkey3: %s\nkey4: %s\n", key, key2, key3, key4);
    }
    
    {
	log("-- Basic config test begins\n");
	
	WvHConf cfg;
	cfg.set("/foo/blah/weasels", "chickens");
	
	cfg["foo"]["pah"]["meatballs"] = 6;
	
	WvHConf &x = cfg["snort/fish/munchkins"];
	x.set("big/bad/weasels", 7);
	x["foo"] = x["blue"] = x["true"] = "sneeze";
	
	log("Config dump:\n");
	cfg.dump(*wvcon);
    }
    
    {
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
	cfg.dump(*wvcon);
    }
    
    {
	log("-- Generator test begins\n");
	
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
	cfg.dump(*wvcon);
    }
    
    return 0;
}
