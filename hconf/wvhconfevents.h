/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHConfEvents is a class that uses the notify/child_notify fields of
 * WvHConf objects to run callback functions automatically.
 */
#ifndef __WVHCONFEVENTS_H
#define __WVHCONFEVENTS_H

#include "wvcallback.h"
#include "wvhconf.h"

// parameters are: userdata, changed HConf object
DeclareWvCallback(2, void, WvHConfCallback, void *, WvHConf &);


class WvHConfEvents
{
public:
    class CallbackInfo
    {
    public:
	WvHConfKey key;
	WvHConfCallback cb;
	void *userdata;
	
	CallbackInfo(WvHConfCallback _cb, void *_userdata, 
		     const WvHConfKey &_key)
	    : key(_key), cb(_cb)
	    { userdata = _userdata; }
    };
    DeclareWvList(CallbackInfo);
    
    WvHConf &cfg;
    CallbackInfoList callbacks;
    
    WvHConfEvents(WvHConf &_cfg);
    
    void do_callbacks();
    void clear_notify();
    
    void add(WvHConfCallback cb, void *userdata, const WvHConfKey &key)
        { callbacks.append(new CallbackInfo(cb, userdata, key), true); }
    void del(WvHConfCallback cb, void *userdata, const WvHConfKey &key);
    
    static void setbool(void *userdata, WvHConf &h);
    
    void add_setbool(bool *b, const WvHConfKey &key)
        { add(setbool, b, key); }
    void del_setbool(bool *b, const WvHConfKey &key)
        { del(setbool, b, key); }
};


#endif // __WVHCONFEVENTS_H
