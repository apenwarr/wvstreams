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

UniConfEvents::NotifierDict UniConfEvents::notifiers(5);

int eventcount = 0;

unsigned int WvHash(const UniConf *ptr)
{
    return WvHash((int)ptr);
}


UniConfEvents::UniConfEvents(UniConf &_cfg, WvStringParm _label)
    : cfg(_cfg), label(_label)
{
    notifier = NULL;
    find_notifier();

}


UniConfEvents::~UniConfEvents()
{
    if (notifier)
	notifier->events.unlink(this);
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
    
    // if this object _or_ anything under it has changed, then we have
    // a match.
    if (h && (h->notify || h->child_notify))
	return h;
    else 
	return NULL;
}

void UniConfEvents::add(UniConfCallback cb, void *userdata, const UniConfKey &key, bool one_shot)
{     
    callbacks.append(new CallbackInfo(cb, userdata, key, one_shot? 1 : 0 ), true); 
    eventcount++;
    WvLog log(label, WvLog::Debug);
//    log("Added a callback for %s.There are now %s events.\n", key, eventcount);
}


// find the closest UniConfNotifier that can contain us.
void UniConfEvents::find_notifier()
{
    UniConf *h;
    Notifier *tmp;
    
    // unregister from old notifier, if any
    if (notifier)
	notifier->events.unlink(this);
    notifier = NULL;
    
    for (h = &cfg; h; h = h->parent)
    {
	tmp = notifiers[h];
	if (tmp)
	{
	    notifier = tmp->notifier;
	    notifier->events.append(this, false);
	    break;
	}
    }
    
    assert(notifier);
}


// run all registered callbacks, then set all the 'notify' flags in the
// UniConf tree back to false.
void UniConfEvents::do_callbacks()
{
    UniConf *h;
    
    CallbackInfoList::Iter i(callbacks);
    WvLog log(label, WvLog::Debug);
    for (i.rewind(); i.next(); )
    {
	h = find_match(&cfg, i->key);
	if (h)
        {
	    i->cb(i->userdata, *h);
        }
        // Now, if it's a one shot event, delete it.
        if (i->one_shot != 0)
            i.xunlink();
    }
}


void UniConfEvents::del(UniConfCallback cb,
			    void *userdata, const UniConfKey &key)
{
    WvLog log(label, WvLog::Debug);
    CallbackInfoList::Iter i(callbacks);
    for (i.rewind(); i.next(); )
    {
	if (i->cb == cb && i->userdata == userdata 
	  && i->key.printable() == key.printable())
        {
            eventcount--;
/*            log("Deleted a callback for %s.  There are now %s events.\n",
                    key.printable(), eventcount);*/
	    i.xunlink();
        }
    }
}


void UniConfEvents::setbool(void *userdata, UniConf &h)
{
    if (!*(bool *)userdata)
    {
	WvLog log(label, WvLog::Debug);
	log("Changed: '%s' = '%s'\n", h.full_key(&cfg), h);
    }
    
    *(bool *)userdata = true;
}


///////////////////////////////// x


UniConfNotifier::UniConfNotifier(UniConf &_cfgtop) : cfgtop(_cfgtop)
{
    UniConfEvents::Notifier *tmp = new UniConfEvents::Notifier;
    tmp->cfgtop = &cfgtop;
    tmp->notifier = this;
    
    UniConfEvents::notifiers.add(tmp, true);
}


UniConfNotifier::~UniConfNotifier()
{
    assert(events.isempty());
    
    UniConfEvents::NotifierDict::Iter i(UniConfEvents::notifiers);
    for (i.rewind(); i.next(); )
    {
	if (i->notifier == this)
	{
	    UniConfEvents::notifiers.remove(i.ptr());
	    break;
	}
    }
}


void UniConfNotifier::run()
{
    UniConfEventsList::Iter i(events);
    for (i.rewind(); i.next(); )
	i->do_callbacks();
    clear();
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


void UniConfNotifier::clear()
{
    cfgtop.notify = false;
    if (cfgtop.child_notify)
	clear_sub(cfgtop);
}
