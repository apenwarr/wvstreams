/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * See wvstreamregistry.h and iwvstreamregistry.h.
 */ 
#include "wvstreamautoregister.h"
#include "wvstreamregistry.h"


IWvStream *new_wvstream(WvStringParm s, IObject *obj, void *userdata)
{
    IWvStreamRegistry *reg = IWvStreamRegistry::get_registry();
    if (reg)
	return reg->create(s, obj, userdata);
    else
	return NULL;
}


#if HAS_XPLC

#include <xplc/IStaticServiceHandler.h>

IWvStreamRegistry *IWvStreamRegistry::get_registry()
{
    IServiceManager *servmgr = XPLC_getServiceManager();
    IWvStreamRegistry *reg = 
	mutate<IWvStreamRegistry>(servmgr->getObject(IWvStreamRegistry_OID));
    
    servmgr->release();
    
    if (reg)
	reg->release(); // assume we won't be keeping the pointer for long
    
    return reg;
}




WvStreamRegistry::WvStreamRegistry()
    : list(10)
{
    refcount = 0;
    
    servmgr = XPLC_getServiceManager();
    
    handler = mutate<IStaticServiceHandler>
		   (servmgr->getObject(XPLC_staticServiceHandler));
    
    handler->addObject(IWvStreamRegistry_OID, this);
    handler->release();
}


WvStreamRegistry::~WvStreamRegistry()
{
    servmgr->release();
}


unsigned int WvStreamRegistry::addRef()
{
    return ++refcount;
}


unsigned int WvStreamRegistry::release()
{
    if (--refcount > 1)
	return refcount;
    
    if (refcount == 1)
    {
	// handler has one reference to us, but it's no longer needed
	handler->removeObject(IWvStreamRegistry_OID);
	     // release() gets called recursively here!!
	return 0;
    }
    
    /* protect against re-entering the destructor */
    refcount = 1;
    delete this;
    return 0;
}


IObject *WvStreamRegistry::getInterface(const UUID &uuid)
{
    if (uuid.equals(IObject_IID))
    {
	addRef();
	return this;
    }
    
    if (uuid.equals(IWvStreamRegistry_IID))
    {
	addRef();
	return this;
    }
    
    return 0;
}


#else // not HAS_XPLC, so we have to fake it (badly!)

IWvStreamRegistry *IWvStreamRegistry::get_registry()
{
    static IWvStreamRegistry *reg;
    
    if (!reg)
	reg = new WvStreamRegistry;
    
    return reg;
}


WvStreamRegistry::WvStreamRegistry()
    : list(10)
{
    refcount = 0;
}


WvStreamRegistry::~WvStreamRegistry()
{
    // nothing to do
}


unsigned int WvStreamRegistry::addRef()
{
    return 1;
}


unsigned int WvStreamRegistry::release()
{
    return 1;
}


IObject *WvStreamRegistry::getInterface(const UUID &uuid)
{
    return NULL;
}


#endif // not HAS_XPLC


IWvStream *WvStreamRegistry::create(WvStringParm _s,
				    IObject *obj, void *userdata)
{
    WvString s(_s);
    char *cptr = strchr(s.edit(), ':');
    if (cptr)
	*cptr++ = 0;
    else
	cptr = "";
    Registration *r = list[s];
    if (r)
    {
	IWvStream *s = r->func(cptr, obj, userdata);
	s->addRef();
	return s;
    }
    else
	return NULL;
}


WvStreamAutoRegister::WvStreamAutoRegister(WvStringParm _id,
				   WvStreamRegistry::CreateFunc *func)
    : id(_id)
{
    reg = IWvStreamRegistry::get_registry();
    
    if (!reg)
	reg = new WvStreamRegistry;
    
    reg->addRef();
    
    if (reg)
	reg->add(id, func);
}


WvStreamAutoRegister::~WvStreamAutoRegister()
{
    if (reg)
    {
	reg->del(id);
	reg->release();
    }
}


