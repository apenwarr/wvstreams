/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen that reconnects to an inner generator whenever the inner
 * generator is no longer OK.
 */
#include "uniretrygen.h"
#include "wvmoniker.h"
#include "wvtclstring.h"
#include "wvstringlist.h"
#include "wvlinkerhack.h"

WV_LINK(UniRetryGen);


#if 0
#define DPRINTF(format, args...) fprintf(stderr, format ,##args);
#else
#define DPRINTF if (0) printf
#endif


// Wrap the given moniker.  For the retry generator, obj is useless since
// it gives us no idea as to how to recreate itself.
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    if (obj)
    	return NULL;
    
    WvString encoded_params = wvtcl_unescape(s);
    DPRINTF("encoded_params = %s\n", encoded_params.cstr());
    WvStringList params;
    wvtcl_decode(params, encoded_params);
    if (params.count() == 0)
    	return NULL;
    	
    WvString moniker = params.popstr();
    if (params.count() == 0)
    	return new UniRetryGen(moniker);
    	
    WvString retry_interval_ms_str = params.popstr();
    time_t retry_interval_ms = retry_interval_ms_str.num();
    if (retry_interval_ms < 0)
    	retry_interval_ms = 0;
    return new UniRetryGen(moniker, retry_interval_ms);
}

static WvMoniker<IUniConfGen> reg("retry", creator);


/***** UniRetryGen *****/

UniRetryGen::UniRetryGen(WvStringParm _moniker,
    	time_t _retry_interval_ms) 
    : UniFilterGen(NULL),
    	log(WvString("UniRetryGen %s", _moniker), WvLog::Debug1),
    	moniker(_moniker),
    	retry_interval_ms(_retry_interval_ms),
    	next_reconnect_attempt(wvtime())
{
    DPRINTF("UniRetryGen::UniRetryGen(%s, %ld)\n",
    	    moniker.cstr(), retry_interval_ms);
    
    maybe_reconnect();
}


void UniRetryGen::maybe_reconnect()
{
    if (!inner())
    {
    	if (!(wvtime() < next_reconnect_attempt))
    	{
    	    IUniConfGen *gen = wvcreate<IUniConfGen>(moniker);
            
    	    if (!gen)
    	    {
    	    	DPRINTF("UniRetryGen::maybe_reconnect: !gen\n");
    	    	return;
    	    }
        
    	    if (gen->isok())
    	    {
    	    	DPRINTF("UniRetryGen::maybe_reconnect: gen->isok()\n");
    	    	
    	    	log("Connected\n");
    	    	
    	    	setinner(gen);
    	    }
    	    else
    	    {
    	    	DPRINTF("UniRetryGen::maybe_reconnect: !gen->isok()\n");
    	    	
    	    	WVRELEASE(gen);
            	
    	    	next_reconnect_attempt =
    	    	    	msecadd(next_reconnect_attempt, retry_interval_ms);
    	    }    	
    	}
    }
}


void UniRetryGen::maybe_disconnect()
{
    if (inner() && !inner()->isok())
    {
    	DPRINTF("UniRetryGen::maybe_disconnect: inner() && !inner()->isok()\n");
    	    	
    	log("Disconnected\n");
    	
    	IUniConfGen *old_inner = inner();
    	
    	setinner(NULL);
    	
    	WVRELEASE(old_inner);

        next_reconnect_attempt = msecadd(wvtime(), retry_interval_ms);
    }
}


void UniRetryGen::commit()
{
    maybe_reconnect();
    
    if (UniFilterGen::isok())
    	UniFilterGen::commit();
    
    maybe_disconnect();
}


bool UniRetryGen::refresh()
{
    maybe_reconnect();
    
    bool result;
    if (UniFilterGen::isok())
    	result = UniFilterGen::refresh();
    else
    	result = false;
    
    maybe_disconnect();
    
    return result;
}


void UniRetryGen::prefetch(const UniConfKey &key, bool recursive)
{
    maybe_reconnect();
    
    if (UniFilterGen::isok())
    	UniFilterGen::prefetch(key, recursive);
    
    maybe_disconnect();
}


WvString UniRetryGen::get(const UniConfKey &key)
{
    maybe_reconnect();
    
    WvString result;
    if (UniFilterGen::isok())
    {
    	result = UniFilterGen::get(key);
    	DPRINTF("UniRetryGen::get(%s) returns %s\n", key.cstr(), result.cstr());
    }
    else
    {
    	DPRINTF("UniRetryGen::get(%s): !isok()\n", key.cstr());
    	result = WvString::null;
    }
    
    maybe_disconnect();

    return result;
}


void UniRetryGen::set(const UniConfKey &key, WvStringParm value)
{
    maybe_reconnect();
    
    if (UniFilterGen::isok())
    	UniFilterGen::set(key, value);
    
    maybe_disconnect();
}


bool UniRetryGen::exists(const UniConfKey &key)
{
    maybe_reconnect();
    
    DPRINTF("UniRetryGen::exists(%s)\n", key.cstr());
    
    bool result;
    if (UniFilterGen::isok())
    {
    	result = UniFilterGen::exists(key);
    	DPRINTF("UniRetryGen::exists: returns %s\n", result? "true": "false");
    }
    else
    {
    	DPRINTF("UniRetryGen::exists: !isok()\n");
    	result = false;
    }
    
    maybe_disconnect();
    
    return result;
}


bool UniRetryGen::haschildren(const UniConfKey &key)
{
    maybe_reconnect();
    
    bool result;
    if (UniFilterGen::isok())
    	result = UniFilterGen::haschildren(key);
    else
    	result = false;
    
    maybe_disconnect();
    
    return result;
}


bool UniRetryGen::isok()
{
    maybe_reconnect();
    
    bool result = UniFilterGen::isok();
    
    maybe_disconnect();
    
    return result;
}


UniConfGen::Iter *UniRetryGen::iterator(const UniConfKey &key)
{
    maybe_reconnect();
    
    Iter *result;
    if (UniFilterGen::isok())
    	result = UniFilterGen::iterator(key);
    else
    	result = NULL;
    
    maybe_disconnect();
    
    return result;
}


UniConfGen::Iter *UniRetryGen::recursiveiterator(const UniConfKey &key)
{
    maybe_reconnect();
    
    Iter *result = UniFilterGen::recursiveiterator(key);
    
    maybe_disconnect();
    
    return result;
}
