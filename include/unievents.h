/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConfEvents is a class that uses the notify/child_notify fields of
 * UniConf objects to run callback functions automatically.
 */
#ifndef __UNICONFEVENTS_H
#define __UNICONFEVENTS_H

#include "wvcallback.h"
#include "uniconf.h"

// parameters are: userdata, changed UniConf object
DeclareWvCallback(2, void, UniConfCallback, void *, UniConf &);


class UniConfEvents;
class UniConfNotifier;

unsigned int WvHash(const UniConf *);


class UniConfEvents
{
private:
    friend class UniConfNotifier;
    
    class CallbackInfo
    {
    public:
	UniConfKey key;
	UniConfCallback cb;
	void *userdata;
        int one_shot:1; // Is this a one shot [self deleting] event?
	
	CallbackInfo(UniConfCallback _cb, void *_userdata, 
		     const UniConfKey &_key, int os=0)
	    : key(_key), cb(_cb), one_shot(os)
	    { userdata = _userdata; }
    };
    DeclareWvList(CallbackInfo);
    
    UniConf &cfg;
    WvString label;
    CallbackInfoList callbacks;
    
    // stuff for tracking the UniConfNotifier object that will call our
    // do_callbacks() function
    struct Notifier
    {
	UniConf *cfgtop;
	UniConfNotifier *notifier;
    };
    DeclareWvDict(Notifier, UniConf *, cfgtop);
    static NotifierDict notifiers;
    UniConfNotifier *notifier;

    // find the best existing UniConfNotifier object that includes our subtree.
    // crashes the program if it can't find one!
    void find_notifier();
    
    // actually call our registered callbacks for any objects that have
    // changed.
    void do_callbacks();
    
public:
    UniConfEvents(UniConf &_cfg, WvStringParm _label = "Config Event");
    ~UniConfEvents();
    
    void add(UniConfCallback cb, void *userdata, const UniConfKey &key, bool one_shot=false);
    void del(UniConfCallback cb, void *userdata, const UniConfKey &key);
    
    
    // convenience functions for auto-setting a bool on an event
    
    void setbool(void *userdata, UniConf &h);
    
    void add_setbool(bool *b, const UniConfKey &key)
        { add(wvcallback(UniConfCallback, *this, UniConfEvents::setbool),
	      b, key); }
    void del_setbool(bool *b, const UniConfKey &key)
        { del(wvcallback(UniConfCallback, *this, UniConfEvents::setbool),
	      b, key); }
};

DeclareWvList(UniConfEvents);


class UniConfNotifier
{
public:
    UniConf &cfgtop;
    UniConfEventsList events;
    
    UniConfNotifier(UniConf &_cfgtop);
    ~UniConfNotifier();
    
    void run();
    void clear();
};


#endif // __UNICONFEVENTS_H
