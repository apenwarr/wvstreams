/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVSTREAMREGISTRY_H
#define __WVSTREAMREGISTRY_H

#include "iwvstreamregistry.h"
#include "wvhashtable.h"
#include "wvlog.h"

#include "wvxplc.h"

class IServiceManager;
class IStaticServiceHandler;

class WvStreamRegistry : public IWvStreamRegistry
{
    struct Registration
    {
	WvString id;
	CreateFunc *func;
	
	Registration(WvStringParm _id, CreateFunc *_func)
	    : id(_id) { func = _func; }
    };
    
    DeclareWvDict(Registration, WvString, id);
    
    RegistrationDict list;
    IServiceManager *servmgr;
    IStaticServiceHandler *handler;
    unsigned int refcount;
    
public:
    WvStreamRegistry();
    virtual ~WvStreamRegistry();
    
    virtual void add(WvStringParm id, CreateFunc *func)
        { list.add(new Registration(id, func), true); }
    virtual void del(WvStringParm id)
        { list.remove(list[id]); }
    
    virtual IWvStream *create(WvStringParm _s,
			      IObject *obj = NULL, void *userdata = NULL);
    
    // IObject stuff
    virtual IObject *getInterface(const UUID &uuid);
    
    // we can't use GenericComponent, since we have to unregister ourselves
    // on the second-last release().
    virtual unsigned int addRef();
    virtual unsigned int release();
};


#endif // __WVSTREAMREGISTRY_H
		    
