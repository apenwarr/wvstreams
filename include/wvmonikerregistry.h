/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for moniker registries.  See wvmoniker.h.
 */
#ifndef __WVMONIKERREGISTRY_H
#define __WVMONIKERREGISTRY_H

#include "wvmoniker.h"
#include "wvscatterhash.h"
#include <xplc/core.h>
#include <xplc/IStaticServiceHandler.h>


/**
 * A dictionary for holding moniker-prefix to factory-function mappings.
 * 
 * This is used by WvMoniker and wvcreate().  See those for details.
 */
class WvMonikerRegistry : public IObject
{
    IMPLEMENT_IOBJECT(WvMonikerRegistry);
    
    struct Registration
    {
	WvString id;
	const UUID &cid;
	WvMonikerCreateFunc *func;
	
	Registration(WvStringParm _id, WvMonikerCreateFunc *_func) 
	    : id(_id), cid(UUID_null)
	    { func = _func; }
	
	Registration(WvStringParm _id, const UUID &_cid)
	    : id(_id), cid(_cid)
	    { func = NULL; }
    };
    
    DeclareWvScatterDict(Registration, WvString, id);

    unsigned refcount;
    
public:
    UUID reg_iid;
    RegistrationDict dict;
    
    WvMonikerRegistry(const UUID &iid);
    virtual ~WvMonikerRegistry();
    
    virtual void add(WvStringParm id, WvMonikerCreateFunc *func);
    virtual void add(WvStringParm id, const UUID &cid);
    virtual void del(WvStringParm id);
    
    virtual void *create(WvStringParm _s,
			 IObject *obj = NULL, void *userdata = NULL);
    
};


class WvRegistryRegistry : public IObject
{
    IMPLEMENT_IOBJECT(WvRegistryRegistry);
    xplc_ptr<IStaticServiceHandler> handler;

    DeclareWvScatterDict(WvMonikerRegistry, UUID, reg_iid);
    WvMonikerRegistryDict regs;
    
    WvMonikerRegistry *_find_reg(const UUID &iid);
public:
    
    WvRegistryRegistry();
    virtual ~WvRegistryRegistry();
    
    /// find a registry for objects of the given interface UUID
    static WvMonikerRegistry *find_reg(const UUID &iid);
};

static const UUID WvRegistryRegistry_CID = {
    0x33f126c6, 0xaad4, 0x4c54,
    {0xa2, 0x33, 0xf9, 0x50, 0x3b, 0xa5, 0x2b, 0x12}
};

#endif // __WVMONIKERREGISTRY_H
