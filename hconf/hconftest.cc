/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for the new, hierarchical WvConf.
 */
#include "wvhconf.h"
#include "wvlog.h"
#include "wvconf.h"

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
	
	log("Config dump 2:\n");
	cfg.dump(*wvcon);
    }
    
    return 0;
}
