/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"
#include "uniconfiter.h"


/***** UniConfGen *****/

UniConfGen::UniConfGen() :
    cbdata(NULL)
{
}


UniConfGen::~UniConfGen()
{
}


void UniConfGen::delta(const UniConfKey &key, UniConfDepth::Type depth)
{
    if (cb)
        cb(*this, key, depth, cbdata);
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


void UniConfGen::setcallback(const UniConfGenCallback &callback,
    void *userdata)
{
    cb = callback;
    cbdata = userdata;
}



/***** UniFilterGen *****/

UniFilterGen::UniFilterGen(UniConfGen *inner) :
    xinner(NULL)
{
    setinner(inner);
}


UniFilterGen::~UniFilterGen()
{
    delete xinner;
}


void UniFilterGen::setinner(UniConfGen *inner)
{
    if (xinner)
        xinner->setcallback(UniConfGenCallback(), NULL);
    xinner = inner;
    if (xinner)
        xinner->setcallback(UniConfGenCallback(this,
            &UniFilterGen::gencallback), NULL);
}


bool UniFilterGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->commit(key, depth);
}


bool UniFilterGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->refresh(key, depth);
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    return xinner->get(key);
}


bool UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    return xinner->set(key, value);
}


bool UniFilterGen::zap(const UniConfKey &key)
{
    return xinner->zap(key);
}


bool UniFilterGen::exists(const UniConfKey &key)
{
    return xinner->exists(key);
}


bool UniFilterGen::haschildren(const UniConfKey &key)
{
    return xinner->haschildren(key);
}


bool UniFilterGen::isok()
{
    return xinner->isok();
}


UniConfGen::Iter *UniFilterGen::iterator(const UniConfKey &key)
{
    return xinner->iterator(key);
}


void UniFilterGen::gencallback(const UniConfGen &gen,
    const UniConfKey &key, UniConfDepth::Type depth, void *userdata)
{
    delta(key, depth);
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
#include "uninullgen.h"
static UniConfGenFactoryRegistration nullreg("null",
    new UniNullGenFactory(), true);

#include "unitempgen.h"
static UniConfGenFactoryRegistration tempreg("temp",
    new UniTempGenFactory(), true);

#include "unireadonlygen.h"
static UniConfGenFactoryRegistration readonlyreg("readonly",
    new UniReadOnlyGenFactory(), true);
    
#include "uniinigen.h"
static UniConfGenFactoryRegistration filereg("ini",
    new UniIniGenFactory(), true);
    
#include "uniclientgen.h"
static UniConfGenFactoryRegistration unixreg("unix",
    new UniClientGenFactory(), true);
static UniConfGenFactoryRegistration tcpreg("tcp",
    new UniClientGenFactory(), true);

#endif
