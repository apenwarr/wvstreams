/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __IWVSTREAMREGISTRY_H
#define __IWVSTREAMREGISTRY_H

#include "wvstring.h"

#include "wvxplc.h"

class IWvStream;

class IWvStreamRegistry : public IObject
{
public:
    IWvStreamRegistry() {}
    virtual ~IWvStreamRegistry() {}
    
    typedef IWvStream *CreateFunc(WvStringParm parms,
				  IObject *obj, void *userdata);
    
    virtual void add(WvStringParm id, CreateFunc *func) = 0;
    virtual void del(WvStringParm id) = 0;
    virtual IWvStream *create(WvStringParm _s,
			      IObject *obj = NULL, void *userdata = NULL) = 0;

    // IObject
    static const UUID IID;
    
    // specific object
    static IWvStreamRegistry *get_registry();
};

DEFINE_IID(IWvStreamRegistry, {0x7fb76e98, 0xb653, 0x43d7,
    {0xb0, 0x56, 0x8b, 0x9d, 0xde, 0x9a, 0xbe, 0x9d}});

static const UUID IWvStreamRegistry_OID = {0x5ca76e98, 0xb653, 0x43d7,
                             {0xb0, 0x56, 0x8b, 0x8d,
                              0xde, 0x9a, 0xbe, 0x9d}};

// shortcut for creating objects from the one-and-only registry
IWvStream *new_wvstream(WvStringParm s, 
		    IObject *obj = NULL, void *userdata = NULL);

#endif // __IWVSTREAMREGISTRY_H
