/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"
#include "uniconfiter.h"


UniConfGen::UniConfGen()
{
}


UniConfGen::~UniConfGen()
{
}


bool UniConfGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return true;
}


bool UniConfGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return true;
}


bool UniConfGen::remove(const UniConfKey &key)
{
    return set(key, WvString::null);
}


bool UniConfGen::exists(const UniConfKey &key)
{
    return ! get(key).isnull();
}


bool UniConfGen::isok()
{
    return true;
}



/***** UniConfFilterGen *****/

UniConfFilterGen::UniConfFilterGen(UniConfGen *inner) :
    xinner(inner)
{
}


UniConfFilterGen::~UniConfFilterGen()
{
    delete xinner;
}


bool UniConfFilterGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->commit(key, depth);
}


bool UniConfFilterGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->refresh(key, depth);
}


WvString UniConfFilterGen::get(const UniConfKey &key)
{
    return xinner->get(key);
}


bool UniConfFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    return xinner->set(key, value);
}


bool UniConfFilterGen::zap(const UniConfKey &key)
{
    return xinner->zap(key);
}


bool UniConfFilterGen::exists(const UniConfKey &key)
{
    return xinner->exists(key);
}


bool UniConfFilterGen::haschildren(const UniConfKey &key)
{
    return xinner->haschildren(key);
}


bool UniConfFilterGen::isok()
{
    return xinner->isok();
}


UniConfGen::Iter *UniConfFilterGen::iterator(const UniConfKey &key)
{
    return xinner->iterator(key);
}


#if 0

/****** Registry Hack *****/

/**
 * This is  a quick hack to ensure that all standard generators
 * are always available.  Unless explicitly referenced by the
 * program, some generators may not be linked with the executable
 * and therefore would not be registered if their static initializers
 * resided in their respective modules only.  This hack should be
 * removed if ever dynamic loading of generators is provided.
 */
#include "uniconfnull.h"
static UniConfGenFactoryRegistration nullreg("null",
    new UniConfNullGenFactory(), true);

#include "uniconftemp.h"
static UniConfGenFactoryRegistration tempreg("temp",
    new UniConfTempGenFactory(), true);

#include "uniconfreadonly.h"
static UniConfGenFactoryRegistration readonlyreg("readonly",
    new UniConfReadOnlyGenFactory(), true);
    
#include "uniconfini.h"
static UniConfGenFactoryRegistration filereg("ini",
    new UniConfIniFileGenFactory(), true);
    
#include "uniconfclient.h"
static UniConfGenFactoryRegistration unixreg("unix",
    new UniConfClientGenFactory(), true);
static UniConfGenFactoryRegistration tcpreg("tcp",
    new UniConfClientGenFactory(), true);

#endif
