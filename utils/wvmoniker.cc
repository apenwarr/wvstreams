/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for monikers, which are strings that you can pass to a magic
 * factory to get objects supporting a particular interface.  See wvmoniker.h.
 */
#include "wvmonikerregistry.h"
#include <assert.h>
#include <stdio.h>

#if 0
# define DEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#else
# define DEBUG(fmt, args...)
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

DeclareWvDict(WvMonikerRegistry, UUID, reg_iid);
static WvMonikerRegistryDict *regs;
  


WvMonikerRegistry::WvMonikerRegistry(const UUID &iid) 
    : reg_iid(iid), dict(10)
{
    DEBUG("WvMonikerRegistry creating.\n");
    refcount = 0;
}


WvMonikerRegistry::~WvMonikerRegistry()
{
    DEBUG("WvMonikerRegistry destroying.\n");
}


void WvMonikerRegistry::add(WvStringParm id, WvMonikerCreateFunc *func)
{
    DEBUG("WvMonikerRegistry register(%s).\n", id.cstr());
    assert(!dict[id]);
    dict.add(new Registration(id, func), true);
}


void WvMonikerRegistry::del(WvStringParm id)
{
    DEBUG("WvMonikerRegistry unregister(%s).\n", id.cstr());
    assert(dict[id]);
    dict.remove(dict[id]);
}


IObject *WvMonikerRegistry::create(WvStringParm _s,
				   IObject *obj = NULL, void *userdata = NULL)
{
    WvString s(_s);
    char *cptr = strchr(s.edit(), ':');
    if (cptr)
	*cptr++ = 0;
    else
	cptr = "";
    
    DEBUG("WvMonikerRegistry create object ('%s' '%s').\n", s.cstr(), cptr);
    
    Registration *r = dict[s];
    if (r)
    {
	IObject *s = r->func(cptr, obj, userdata);
	s->addRef();
	return s;
    }
    else
	return NULL;
}


WvMonikerRegistry *WvMonikerRegistry::find_reg(const UUID &iid)
{
    DEBUG("WvMonikerRegistry find_reg.\n");
    
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
    DEBUG("WvMonikerRegistry addRef.\n");
    return ++refcount;
}


unsigned int WvMonikerRegistry::release()
{
    DEBUG("WvMonikerRegistry release.\n");
    
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
    DEBUG("WvMoniker creating(%s).\n", id.cstr());
    reg = WvMonikerRegistry::find_reg(iid);
    if (reg)
	reg->add(id, func);
}


WvMonikerBase::~WvMonikerBase()
{
    DEBUG("WvMoniker destroying(%s).\n", id.cstr());
    if (reg)
    {
	reg->del(id);
	reg->release();
    }
}


IObject *wvcreate(const UUID &iid,
		  WvStringParm s, IObject *obj, void *userdata)
{
    WvMonikerRegistry *reg = WvMonikerRegistry::find_reg(iid);
    IObject *o = reg->create(s, obj, userdata);
    reg->release();
    return o;
}
