#ifndef __WVPLUGIN_H
#define __WVPLUGIN_H

#include "wvmoniker.h"
#include <xplc/module.h>

/**
 * A "factory" object that can create an object given a moniker.  This is
 * needed because in XPLC, IModule can only get components from a CID, not
 * functions, but we need to pass parameters to the moniker creation
 * function before we can get the *real* object we're looking for.
 * 
 * Thus, when you want to resolve a moniker, you find the CID corresponding
 * to that moniker, then you request the object for that CID (which must
 * support IWvMonikerFactory), and then you call create() on that object
 * to get the object you actually wanted.
 * 
 * This sounds complicated, but see wvcreate(), which does all this to
 * create objects for you, and WvMoniker, which sets up the chain of objects
 * so that wvcreate() can work.
 */
class IWvMonikerFactory : public IObject
{
public:
    virtual void *create(WvStringParm s, IObject *obj, void *userdata) = 0;
};

DEFINE_IID(IWvMonikerFactory, {
    0x584cffb3, 0xa78d, 0x44f2,
    {0xa8, 0xbc, 0xfc, 0xfd, 0x3b, 0x30, 0x07, 0x35}
});


/**
 * The standard concrete class implementing IWvMonikerFactory.  It's very
 * simple.  You give it a factory function in the constructor, and then when
 * anybody calls create(), it calls that factory function.
 */
class WvMonikerFactory : public IWvMonikerFactory
{
    IMPLEMENT_IOBJECT(WvMonikerFactory);
    WvMonikerCreateFunc *func;
public:
    WvMonikerFactory(WvMonikerCreateFunc *_func);
    virtual void *create(WvStringParm s, IObject *obj, void *userdata);
};


/**
 * A ridiculous templated function needed because the 'components' member
 * of XPLC_ModuleInfo gives only functions that take no parameters.  Thus,
 * we have to generate functions on the fly that will actually generate
 * appropriate factory objects.  Used by WVPLUGIN().
 */
template <WvMonikerCreateFunc func>
static IObject *_wv_create_func()
{
    return new WvMonikerFactory(func);
}


/**
 * A class that we can construct in order to make sure functions get
 * called appropriately during the creation of the XPLC_ComponentEntry array.
 * This is pretty icky, but necessary for our "nice" WVPLUGIN macros to work.
 * 
 * This class doesn't do much.  You usually want the templatized version,
 * WvPluginEntry, which also registers a moniker for your object.
 */
class WvPluginEntryBase : public XPLC_ComponentEntry
{
public:
    WvPluginEntryBase(const UUID &_cid, IObject* (*_getObject)());
};


/**
 * A class that we can use in place of XPLC_ComponentEntry that registers
 * a WvMoniker name->cid mapping as well as the usual cid->function mapping.
 */
template <typename T>
class WvPluginEntry : public WvPluginEntryBase
{
public:
    WvPluginEntry(const char *name,
		  const UUID &_cid, IObject* (*_getObject)())
	: WvPluginEntryBase(_cid, _getObject)
    {
	// WvMoniker for 'cid' never unregisters itself, so we don't have to
	// keep this object around.
	WvMoniker<T>(name, _cid);
    }
};


#define WVPLUGIN_START(description) \
    static const char _wv_description[] = description; \
    static const XPLC_ComponentEntry _wv_components[] = {
#define WVPLUGIN(ifc, name, cid, func) \
        WvPluginEntry<ifc>(name, cid, _wv_create_func<&func>),
#define WVPLUGIN_END \
        {UUID_null, 0} \
    }; \
    const XPLC_ModuleInfo XPLC_Module = { \
	magic: XPLC_MODULE_MAGIC, \
	version_major: XPLC_MODULE_VERSION_MAJOR, \
	version_minor: XPLC_MODULE_VERSION_MINOR, \
	description: _wv_description, \
	components: _wv_components, \
	loadModule: 0, \
	unloadModule: 0, \
    };


#endif // __WVPLUGIN_H
