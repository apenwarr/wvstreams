/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfEvents is a class that uses the notify/child_notify fields of
 * UniConf objects to run callback functions automatically.
 */
#include "unievents.h"
#include "uniconfiter.h"
#include "wvlog.h"


UniConfEvents::UniConfEvents(UniConf &_cfg)
    : cfg(_cfg)
{
    // nothing to do
}


static UniConf *find_match(UniConf *h, const UniConfKey &key)
{
    UniConf *h2;
    
    UniConfKey::Iter ki(key);
    for (ki.rewind(); h && ki.next(); )
    {
	if (*ki == "*") // wildcard
	{
	    UniConf::Iter i(*h);
	    for (i.rewind(); i.next(); )
	    {
		h2 = find_match(i.ptr(), key.skip(1));
		if (h2)
		    return h2; // found a match somewhere inside the wildcard
	    }
	}
	else // just walk down another level
	{
	    if (!h->child_notify)
		return NULL; // no matches below this level
	    h = h->find(*ki);
	}
    }
    
    if (h && h->notify)
	return h;
    else 
	return NULL;
}


// run all registered callbacks, then set all the 'notify' flags in the
// HConf tree back to false.
void UniConfEvents::do_callbacks()
{
    UniConf *h;
    
    CallbackInfoList::Iter i(callbacks);
    for (i.rewind(); i.next(); )
    {
	h = find_match(&cfg, i->key);
	if (h)
	    i->cb(i->userdata, *h);
    }
    
    clear_notify();
}


static void clear_sub(UniConf &h)
{
    h.child_notify = false;
    
    UniConf::Iter i(h);
    for (i.rewind(); i.next(); )
    {
	i->notify = false;
	if (i->child_notify)
	    clear_sub(*i);
    }
}


void UniConfEvents::clear_notify()
{
    cfg.notify = false;
    if (cfg.child_notify)
	clear_sub(cfg);
}


void UniConfEvents::del(UniConfCallback cb,
			    void *userdata, const UniConfKey &key)
{
    CallbackInfoList::Iter i(callbacks);
    for (i.rewind(); i.next(); )
    {
	if (i->cb == cb && i->userdata == userdata 
	  && i->key.printable() == key.printable())
	    i.xunlink();
    }
}


void UniConfEvents::setbool(void *userdata, UniConf &h)
{
    if (!*(bool *)userdata)
    {
	WvLog log("Config Event", WvLog::Debug);
	log("Changed: '%s' = '%s'\n", h.full_key(), h);
    }
    
    *(bool *)userdata = true;
}


