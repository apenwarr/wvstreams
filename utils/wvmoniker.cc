/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for monikers, which are strings that you can pass to a magic
 * factory to get objects supporting a particular interface.  See wvmoniker.h.
 */
#include "wvplugin.h"
#include "wvmonikerregistry.h"
#include "strutils.h"
#include "wvhashtable.h"
#include <assert.h>
#include <stdio.h>

#if 0 
# define DEBUGLOG(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#ifndef _MSC_VER
# define DEBUGLOG(fmt, args...)
#else  // MS Visual C++ doesn't support varags preproc macros
# define DEBUGLOG
#endif
#endif


UUID_MAP_BEGIN(WvMonikerRegistry)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_END

UUID_MAP_BEGIN(WvRegistryRegistry)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_END

UUID_MAP_BEGIN(WvMonikerFactory)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IWvMonikerFactory)
  UUID_MAP_END

  

static unsigned WvHash(const UUID &_uuid)
{
    unsigned val = 0;
    unsigned int *uuid = (unsigned int *)&_uuid;
    int max = sizeof(UUID)/sizeof(*uuid);
    
    for (int count = 0; count < max; count++)
	val += uuid[count];
    
    return val;
}


WvMonikerFactory::WvMonikerFactory(WvMonikerCreateFunc *_func)
{
    func = _func;
}


void *WvMonikerFactory::create(WvStringParm s, IObject *obj, void *userdata)
{
    return func(s, obj, userdata);
}


WvMonikerRegistry::WvMonikerRegistry(const UUID &iid) 
    : reg_iid(iid), dict(10)
{
    DEBUGLOG("WvMonikerRegistry creating.\n");
    refcount = 0;
}


WvMonikerRegistry::~WvMonikerRegistry()
{
    DEBUGLOG("WvMonikerRegistry destroying.\n");
}


void WvMonikerRegistry::add(WvStringParm id, WvMonikerCreateFunc *func)
{
    DEBUGLOG("WvMonikerRegistry register(%s).\n", id.cstr());
    assert(!dict[id]);
    dict.add(new Registration(id, func), true);
}


void WvMonikerRegistry::add(WvStringParm id, const UUID &cid)
{
    DEBUGLOG("WvMonikerRegistry register(%s).\n", id.cstr());
    assert(!dict[id] || dict[id]->cid == cid);
    dict.add(new Registration(id, cid), true);
}


void WvMonikerRegistry::del(WvStringParm id)
{
    DEBUGLOG("WvMonikerRegistry unregister(%s).\n", id.cstr());
    assert(dict[id]);
    dict.remove(dict[id]);
}


void *WvMonikerRegistry::create(WvStringParm _s, IObject *obj, void *userdata)
{
    WvString t(_s);
    WvString s(trim_string(t.edit()));

    char *cptr = strchr(s.edit(), ':');
    if (cptr)
	*cptr++ = 0;
    else
	cptr = "";
    
    DEBUGLOG("WvMonikerRegistry create object ('%s' '%s').\n", s.cstr(), cptr);
    
    Registration *r = dict[s];
    if (r && r->func) // registration has a plain function pointer
	return r->func(cptr, obj, userdata);
    else if (r) // registration uses an object id
    {
	XPLC xplc;
	xplc_ptr<IWvMonikerFactory> factory(
			    xplc.get<IWvMonikerFactory>(r->cid));
	if (factory)
	    return factory->create(cptr, obj, userdata);
	else
	    return NULL;
    }
    else
	return NULL;
}


WvRegistryRegistry::WvRegistryRegistry() : regs(10)
{
    XPLC xplc;
    handler = xplc.get<IStaticServiceHandler>(XPLC_staticServiceHandler);
    if (handler)
	handler->addObject(WvRegistryRegistry_CID, this);
}


WvRegistryRegistry::~WvRegistryRegistry()
{
    if (handler)
	handler->removeObject(WvRegistryRegistry_CID);
}


WvMonikerRegistry *WvRegistryRegistry::_find_reg(const UUID &iid)
{
    DEBUGLOG("WvMonikerRegistry find_reg.\n");
    
    WvMonikerRegistry *reg = regs[iid];
    if (!reg)
    {
	// we have to make one!
	reg = new WvMonikerRegistry(iid);
	regs.add(reg, true);
	reg->addRef(); // one reference for being in the list at all
    }
    
    reg->addRef();
    return reg;
}


WvMonikerRegistry *WvRegistryRegistry::find_reg(const UUID &iid)
{
    XPLC xplc;
    xplc_ptr<WvRegistryRegistry> regs(
	      (WvRegistryRegistry *)xplc.get(WvRegistryRegistry_CID));
    if (!regs)
    {
	WvRegistryRegistry *r = new WvRegistryRegistry;
	r->addRef(); // make sure it stays around
	regs = r;
    }
    return regs->_find_reg(iid);
}


WvMonikerBase::WvMonikerBase(const UUID &iid, WvStringParm _id, 
			     WvMonikerCreateFunc *func)
    : id(_id)
{
    DEBUGLOG("WvMoniker creating func(%s).\n", id.cstr());
    reg = WvRegistryRegistry::find_reg(iid);
    if (reg)
	reg->add(id, func);
}


WvMonikerBase::WvMonikerBase(const UUID &iid, WvStringParm _id, 
			     const UUID &cid)
    : id(_id)
{
    DEBUGLOG("WvMoniker creating cid(%s).\n", id.cstr());
    reg = WvRegistryRegistry::find_reg(iid);
    if (reg)
    {
	reg->add(id, cid);
	
	// if we registered a cid, we don't need to unregister later, so 
	// release the registry right now.  There's always some way to go
	// get a component by cid, even if a loadable module unloads
	// temporarily.  So we'll keep the registration around and xplc will
	// do the work of finding us again later (we hope...)
	RELEASE(reg);
	reg = NULL;
    }
}


WvMonikerBase::~WvMonikerBase()
{
    DEBUGLOG("WvMoniker destroying(%s).\n", id.cstr());
    if (reg)
    {
	reg->del(id);
	RELEASE(reg);
    }
}


WvPluginEntryBase::WvPluginEntryBase(const UUID &_cid,
				     IObject* (*_getObject)())
{
    // stupid XPLC_ComponentEntry is non-assignable due to uninitialized
    // references.  Argh.
    XPLC_ComponentEntry stupid = {_cid, _getObject};
    memcpy((XPLC_ComponentEntry *)this, &stupid, sizeof(stupid));
}


void *wvcreate(const UUID &iid,
	       WvStringParm moniker, IObject *obj, void *userdata)
{
    assert(!moniker.isnull());
    WvMonikerRegistry *reg = WvRegistryRegistry::find_reg(iid);
    if (reg)
    {
	void *ret = reg->create(moniker, obj, userdata);
	RELEASE(reg);
	return ret;
    }
    else
	return NULL;
}
