/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */
 
/** /file
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"
#include "uniconf.h"


/***** UniConfGen *****/

UniConfGen::UniConfGen(const UniConfLocation &location) :
    _location(location)
{
}


UniConfGen::~UniConfGen()
{
}


UniConf *UniConfGen::make_tree(UniConf *parent, const UniConfKey &key)
{
    int segments = key.numsegments();
    for (int i = 0; i < segments; ++i)
    {
        UniConf *child = parent->find(key.segment(i));
        if (!child)
	{
	    child = new UniConf(parent, key.segment(i));
            child->waiting = true;
	    pre_get(child);
	}
	
	parent = child;
    }
 
    update(parent);
    return parent;
}

void UniConfGen::enumerate_subtrees(UniConf *conf, bool recursive)
{
    // do nothing by default.
}

void UniConfGen::pre_get(UniConf *&h)
{
    // do nothing by default.
}

void UniConfGen::update_all()
{
    // do nothing 
}

void UniConfGen::update(UniConf *&h)
{
    // do nothing by default.
    h->dirty = false;
    h->waiting = false;
    h->obsolete = false;
}


void UniConfGen::load()
{
    // do nothing by default
}


void UniConfGen::save()
{
    // do nothing by default
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
    const UniConfLocation &location, UniConf *top)
{
    UniConfGenFactory *factory = find(location.proto());
    if (! factory)
        return NULL;
    return factory->newgen(location, top);
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
