/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfEvents is a class that uses the notify/child_notify fields of
 * UniConf objects to run callback functions automatically.
 */
#ifndef __UNICONFEVENTS_H
#define __UNICONFEVENTS_H

#include "wvcallback.h"
#include "uniconf.h"

// parameters are: userdata, changed UniConf object
DeclareWvCallback(2, void, UniConfCallback, void *, UniConf &);


class UniConfEvents
{
private:
    class CallbackInfo
    {
    public:
	UniConfKey key;
	UniConfCallback cb;
	void *userdata;
	
	CallbackInfo(UniConfCallback _cb, void *_userdata, 
		     const UniConfKey &_key)
	    : key(_key), cb(_cb)
	    { userdata = _userdata; }
    };
    DeclareWvList(CallbackInfo);
    
    UniConf &cfg;
    CallbackInfoList callbacks;
    
public:
    UniConfEvents(UniConf &_cfg);
    
    void do_callbacks();
    void clear_notify();
    
    void add(UniConfCallback cb, void *userdata, const UniConfKey &key)
        { callbacks.append(new CallbackInfo(cb, userdata, key), true); }
    void del(UniConfCallback cb, void *userdata, const UniConfKey &key);
    
    static void setbool(void *userdata, UniConf &h);
    
    void add_setbool(bool *b, const UniConfKey &key)
        { add(setbool, b, key); }
    void del_setbool(bool *b, const UniConfKey &key)
        { del(setbool, b, key); }
};


#endif // __UNICONFEVENTS_H
