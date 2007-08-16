/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A UniConf moniker that uses an .ini file to look up which moniker it
 * should use to find the config file/subtree for a particular application.
 */
#include "uniconfroot.h"
#include "unisubtreegen.h"
#include "wvlinkerhack.h"

WV_LINK(UniAutoGen);


/**
 * The moniker from which the auto: moniker retrieves its own settings.
 * It's a bit silly to override this (since the point is to autoconfigure,
 * and manually configuring the autoconfiguration thing is silly) but it's
 * useful for writing unit tests.
 */
WvString uniautogen_moniker("default:ini:/etc/uniconf.conf");

/*
 * A moniker for finding the "right" config generator for a particular
 * application, given the application name.
 * 
 * For example, for moniker "auto:org/gnome/Nautilus", we would:
 * 
 *  - open /etc/uniconf.conf.
 *  - look for org/gnome/Nautilus in there.
 *    - if it exists, use that value as the config moniker, and return.
 *  - else, look for org/gnome
 *    - if it exists, go get that config moniker,  take the subtree 
 *       "Nautilus" from there, and return.
 *  - else, look for org
 *    - if it exists, go get that config moniker,  take the subtree 
 *       "gnome/Nautilus" from there, and return.
 *  - else, look for /
 *    - if it exists, go get that config moniker,  take the subtree 
 *       "org/gnome/Nautilus" from there, and return.
 *  - else, return a null: generator.
 */
static IUniConfGen *creator(WvStringParm s, IObject *_obj)
{
    UniConfRoot cfg((UniConfGen *)
		    wvcreate<IUniConfGen>(uniautogen_moniker, _obj), true);
    const UniConfKey appname(s);
    
    for (int i = appname.numsegments(); i >= 0; i--)
    {
	UniConfKey prefix(appname.first(i)), suffix(appname.removefirst(i));

	if (!!cfg.xget(prefix))
	{
	    return new UniSubtreeGen(wvcreate<IUniConfGen>(cfg.xget(prefix)),
				     suffix);
	}
    }
    
    return wvcreate<IUniConfGen>("null:");
}


static WvMoniker<IUniConfGen> autoreg("auto", creator);
