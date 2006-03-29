/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for monikers, which are strings that you can pass to a magic
 * factory to get objects supporting a particular interface.  See wvmoniker.h.
 */
#include "wvmonikerregistry.h"
#include "strutils.h"
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


static unsigned WvHash(const UUID &_uuid)
{
    unsigned val = 0;
    unsigned int *uuid = (unsigned int *)&_uuid;
    int max = sizeof(UUID)/sizeof(*uuid);
    
    for (int count = 0; count < max; count++)
	val += uuid[count];
    
    return val;
}


DeclareWvScatterDict(WvMonikerRegistry, UUID, reg_iid);
static WvMonikerRegistryDict *regs;
  


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


void WvMonikerRegistry::del(WvStringParm id)
{
    DEBUGLOG("WvMonikerRegistry unregister(%s).\n", id.cstr());
    assert(dict[id]);
    dict.remove(dict[id]);
}


void *WvMonikerRegistry::create(WvStringParm _s)
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
    if (r)
	return r->func(cptr);
    else
	return NULL;
}


WvMonikerRegistry *WvMonikerRegistry::find_reg(const UUID &iid)
{
    DEBUGLOG("WvMonikerRegistry find_reg.\n");
    
    if (!regs)
	regs = new WvMonikerRegistryDict(10);
    
    WvMonikerRegistry *reg = (*regs)[iid];
    
    if (!reg)
    {
	// we have to make one!
	reg = new WvMonikerRegistry(iid);
	regs->add(reg, true);
	reg->addRef(); // one reference for being in the list at all
    }
    
    reg->addRef();
    return reg;
}


IObject *WvMonikerRegistry::getInterface(const UUID &uuid)
{
#if 0
    if (uuid.equals(IObject_IID))
    {
	addRef();
	return this;
    }
#endif
    
    // we don't really support any interfaces for now.
    
    return 0;
}


unsigned int WvMonikerRegistry::addRef()
{
    DEBUGLOG("WvMonikerRegistry addRef.\n");
    return ++refcount;
}


unsigned int WvMonikerRegistry::release()
{
    DEBUGLOG("WvMonikerRegistry release.\n");
    
    if (--refcount > 1)
	return refcount;
    
    if (refcount == 1)
    {
	// the list has one reference to us, but it's no longer needed.
	// Note: remove() will delete this object!
	regs->remove(this);
	if (regs->isempty())
	{
	    delete regs;
	    regs = NULL;
	}
	return 0;
    }
    
    /* protect against re-entering the destructor */
    refcount = 1;
    delete this;
    return 0;
}


WvMonikerBase::WvMonikerBase(const UUID &iid, WvStringParm _id, 
			     WvMonikerCreateFunc *func)
    : id(_id)
{
    DEBUGLOG("WvMoniker creating(%s).\n", id.cstr());
    reg = WvMonikerRegistry::find_reg(iid);
    if (reg)
	reg->add(id, func);
}


WvMonikerBase::~WvMonikerBase()
{
    DEBUGLOG("WvMoniker destroying(%s).\n", id.cstr());
    if (reg)
    {
	reg->del(id);
	WVRELEASE(reg);
    }
}


void *wvcreate(const UUID &iid, WvStringParm moniker)
{
    assert(!moniker.isnull());
    WvMonikerRegistry *reg = WvMonikerRegistry::find_reg(iid);
    if (reg)
    {
	void *ret = reg->create(moniker);
	WVRELEASE(reg);
	return ret;
    }
    else
	return NULL;
}
