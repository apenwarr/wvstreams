/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for monikers, which are strings that you can pass to a magic
 * factory to get objects supporting a particular interface, without actually
 * knowing anything about the constructor for those objects.
 */
#ifndef __WVMONIKER_H
#define __WVMONIKER_H

#include "wvstring.h"
#include "wvxplc.h"

typedef void *WvMonikerCreateFunc(WvStringParm parms);

/**
 * WvMonikerCreateFuncStore is an IObject that gets registered with
 * XPLC, so that you can have a WvMonikerCreateFunc that creates a new
 * object.
 */
class WvMonikerCreateFuncStore : public IObject
{
    IMPLEMENT_IOBJECT(WvMonikerCreateFuncStore);
    WvMonikerCreateFunc *func;

public:
    WvMonikerCreateFuncStore(WvMonikerCreateFunc *_func)
	: func(_func)
    {
    }

    virtual ~WvMonikerCreateFuncStore()
    {
    }

    void *create(WvStringParm parms)
    {
	return func(parms);
    }
};



/**
 * WvMonikerBase is an auto-registration class for putting things into
 * XPLC.  When a WvMonikerBase instance is created, it registers a
 * moniker prefix ("test:", "ssl:", "ini:", etc) and a factory
 * function that can be used to create an IObject using that prefix.
 *
 * When the instance is destroyed, it auto-unregisters the moniker prefix
 * from XPLC.
 *
 * You can't actually create one of these, because it's not typesafe.  See
 * WvMoniker<T> instead.
 */
class WvMonikerBase
{
protected:
    WvMonikerBase(const UUID &category, WvStringParm _id,
		  const UUID &iid, WvMonikerCreateFunc *func);
    ~WvMonikerBase();

public:
    UUID oid;
    WvMonikerCreateFuncStore func;
};


/**
 * A type-safe version of WvMonikerBase that lets you provide create functions
 * for object types other than IObject.  (The objects themselves have to
 * be derived from IObject, however.)
 *
 * See WvMonikerBase for details.
 *
 * Example:
 *    static IWvStream *createfunc(WvStringParm s)
 *    {
 *        return new WvStream;
 *    }
 *
 *    static WvMoniker<IWvStream> registration("ssl", createfunc);
 */
template <class T>
class WvMoniker : public WvMonikerBase
{
public:
    typedef T *CreateFunc(WvStringParm parms);
    
    WvMoniker(WvStringParm _id, const UUID &_oid, CreateFunc *_func)
	: WvMonikerBase(XPLC_IID<T>::get(), _id, _oid,
			reinterpret_cast<WvMonikerCreateFunc *>(_func))
    { 
	// this looks pointless, but it ensures that T* can be safely,
	// automatically downcast to IObject*.  That means T is really derived
	// from IObject, which is very important. The 'for' avoids a
	// warning.
	for(IObject *silly = (T *)NULL; silly; )
            ;
    };
};


/**
 * Create an object registered in a WvMonikerRegistry.  The iid specifies
 * which registry to look in, and s, obj, and userdata are the parameters to
 * supply to the object's factory.  Most factories need only 's', which is the
 * moniker itself.
 * 
 * Most people don't use this function.  See the templated, type-safe version
 * of wvcreate() below.
 */
void *wvcreate(const UUID &iid, WvStringParm s);


/**
 * Create an object registered in a WvMonikerRegistry.  Exactly which
 * registry is determined by the template type T.
 * 
 * s, obj, and userdata are the parameters to supply to the object's
 * factory.  Most factories need only 's', which is the moniker itself.
 * 
 * Example:
 *    IWvStream *s = wvcreate<IWvStream>("tcp:localhost:25");
 *    IWvStream *s_ssl = wvcreate<IWvStream>("ssl:", s);
 */
template <class T>
inline T *wvcreate(WvStringParm s)
{
    return (T *)(wvcreate(XPLC_IID<T>::get(), s));
}


#endif // __WVMONIKER_H
