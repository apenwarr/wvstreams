/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */
 
/** /file
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"
#include "uniconfiter.h"


/***** UniConfGen *****/

UniConfGen::UniConfGen()
{
}


UniConfGen::~UniConfGen()
{
}


bool UniConfGen::commit(const UniConfKey &key, UniConf::Depth depth)
{
    return true;
}


bool UniConfGen::refresh(const UniConfKey &key, UniConf::Depth depth)
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


void UniConfGen::attach(WvStreamList *streamlist)
{
}


void UniConfGen::detach(WvStreamList *streamlist)
{
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


UniConfLocation UniConfFilterGen::location() const
{
    return xinner->location();
}


bool UniConfFilterGen::commit(const UniConfKey &key,
    UniConf::Depth depth)
{
    return xinner->commit(key, depth);
}


bool UniConfFilterGen::refresh(const UniConfKey &key,
    UniConf::Depth depth)
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


void UniConfFilterGen::attach(WvStreamList *streamlist)
{
    xinner->attach(streamlist);
}


void UniConfFilterGen::detach(WvStreamList *streamlist)
{
    xinner->detach(streamlist);
}


bool UniConfFilterGen::isok()
{
    return xinner->isok();
}


UniConfGen::Iter *UniConfFilterGen::iterator(const UniConfKey &key)
{
    return xinner->iterator(key);
}



/***** UniConfGenFactory *****/

UniConfGenFactory::UniConfGenFactory()
{
}


UniConfGenFactory::~UniConfGenFactory()
{
}



/***** UniConfGenFactoryRegistration *****/

UniConfGenFactoryRegistration::UniConfGenFactoryRegistration(
    WvStringParm _proto, UniConfGenFactory *_factory, bool _autofree) :
    proto(_proto), factory(_factory), autofree(_autofree)
{
    UniConfGenFactoryRegistry::instance()->add(this);
}


UniConfGenFactoryRegistration::~UniConfGenFactoryRegistration()
{
    UniConfGenFactoryRegistry::instance()->remove(this);
    if (autofree)
        delete factory;
}



/***** UniConfGenFactoryRegistry *****/

UniConfGenFactoryRegistry *
    UniConfGenFactoryRegistry::singleton = NULL;


UniConfGenFactoryRegistry *UniConfGenFactoryRegistry::instance()
{
    if (! singleton)
        singleton = new UniConfGenFactoryRegistry();
    return singleton;
}


UniConfGenFactoryRegistry::UniConfGenFactoryRegistry() :
    dict(13), log("UniConfGenFactoryRegistry", WvLog::Debug4)
{
}


UniConfGenFactoryRegistry::~UniConfGenFactoryRegistry()
{
    singleton = NULL;
}


void UniConfGenFactoryRegistry::add(
    UniConfGenFactoryRegistration *reg)
{
    //log("Registering \"%s\"\n", reg->proto);
    dict.add(reg, false);
}


void UniConfGenFactoryRegistry::remove(
    UniConfGenFactoryRegistration *reg)
{
    //log("Unregistering \"%s\"\n", reg->proto);
    dict.remove(reg);
    if (dict.isempty())
        delete this;
}


UniConfGenFactory *UniConfGenFactoryRegistry::find(WvStringParm proto)
{
    UniConfGenFactoryRegistration *reg = dict[proto];
    if (! reg)
    {
        log("Could not find factory for \"%s\"\n", proto);
        return NULL;
    }
    return reg->factory;
}


UniConfGen *UniConfGenFactoryRegistry::newgen(
    const UniConfLocation &location)
{
    UniConfGenFactory *factory = find(location.proto());
    if (! factory)
        return NULL;
    return factory->newgen(location);
}



/****** Registry Hack *****/

/**
 * This is  a quick hack to ensure that all standard generators
 * are always available.  Unless explicitly reference by the
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
