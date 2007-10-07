/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf generator that executes callbacks to generate the value of keys
 */
#ifndef __UNICALLBACKGEN_H
#define __UNICALLBACKGEN_H

#include "unitempgen.h"
#include "wvhashtable.h"
#include "wvstream.h"
#include "wvtr1.h"

typedef wv::function<WvString(const UniConfKey&)>
        UniCallbackGenGetCallback;
typedef wv::function<void(const UniConfKey&, WvStringParm)>
        UniCallbackGenSetCallback;

/**
 * A UniConf generator that executes callbacks to generate the value of keys
 *
 * To make the callback fire and set the value of the key,
 * call set(key, whatever).  Calling get(key) returns the most recent
 * generated value of the key.
 */
class UniCallbackGen : public UniTempGen
{
    WvMap<UniConfKey, UniCallbackGenGetCallback> get_callbacks;
    WvMap<UniConfKey, UniCallbackGenSetCallback> set_callbacks;

public:
    
    bool update_before_get;
    bool update_after_set;

    UniCallbackGen(int size) :
        get_callbacks(size),
        set_callbacks(size),
        update_before_get(false),
        update_after_set(true) {}
    virtual ~UniCallbackGen() {}

    virtual void setgetcallback(const UniConfKey &key,
            UniCallbackGenGetCallback get_callback)
    {
        if (!!get_callback)
            get_callbacks.set(key, get_callback);
        else
            get_callbacks.remove(key);
    }
    virtual void setsetcallback(const UniConfKey &key,
            UniCallbackGenSetCallback set_callback)
    {
        if (!!set_callback)
            set_callbacks.set(key, set_callback);
        else
            set_callbacks.remove(key);
    }

    virtual void update(const UniConfKey &key, 
            WvStringParm value = WvString::null)
    {
        if (get_callbacks.exists(key))
        {
            UniCallbackGenGetCallback &get_callback = get_callbacks[key];
            UniTempGen::set(key, get_callback(key));
        }
        else UniTempGen::set(key, value);
    }

    /***** Overridden members *****/
    virtual WvString get(const UniConfKey &key)
    {
        if (update_before_get) update(key);

        return UniTempGen::get(key);
    }
    virtual void set(const UniConfKey &key, WvStringParm value)
    {
        if (set_callbacks.exists(key))
        {
            UniCallbackGenSetCallback &set_callback = set_callbacks[key];
            set_callback(key, value);
        }

        if (update_after_set) update(key, value);
    }
};


#endif // __UNICALLBACKGEN_H
