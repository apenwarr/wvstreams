/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVSTREAMAUTOREGISTER_H
#define __WVSTREAMAUTOREGISTER_H

#include "iwvstreamregistry.h"

class WvStreamAutoRegister
{
public:
    WvString id;
    IWvStreamRegistry *reg;
    
    WvStreamAutoRegister(WvStringParm _id,
			 IWvStreamRegistry::CreateFunc *func);
    ~WvStreamAutoRegister();
};


#endif // __WVSTREAMAUTOREGISTER_H
