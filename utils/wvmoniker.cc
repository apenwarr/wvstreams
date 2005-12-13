/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Support for monikers, which are strings that you can pass to a magic
 * factory to get objects supporting a particular interface.  See wvmoniker.h.
 */
#include "wvmonikerregistry.h"
#include "strutils.h"

#include "xplc/core.h"
#include "xplc/ICategory.h"
#include "xplc/ICategoryIterator.h"
#include "xplc/ICategoryManager.h"
#include "xplc/IStaticServiceHandler.h"

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


UUID_MAP_BEGIN(WvMonikerCreateFuncStore)
  UUID_MAP_ENTRY(IObject)
UUID_MAP_END


WvMonikerBase::WvMonikerBase(const UUID &iid, WvStringParm id,
			     const UUID &_oid, WvMonikerCreateFunc *_func)
  : oid(_oid), func(_func)
{
    DEBUGLOG("WvMoniker creating(%s).\n", id.cstr());

    IServiceManager *srvmgr = XPLC_getServiceManager();
    if (srvmgr)
    {
	ICategoryManager *catmgr;
	catmgr = static_cast<ICategoryManager *>(
	    srvmgr->getObject(XPLC_categoryManager));
	IStaticServiceHandler *ssrvhdlr;
	ssrvhdlr = static_cast<IStaticServiceHandler *>(
	    srvmgr->getObject(XPLC_staticServiceHandler));
	if (catmgr && ssrvhdlr)
	{
	    catmgr->registerComponent(iid, oid, id);
	    ssrvhdlr->addObject(oid, &func);
	}
    }
}


WvMonikerBase::~WvMonikerBase()
{
    DEBUGLOG("WvMoniker destroying(%s).\n", id.cstr());
    IServiceManager *srvmgr = XPLC_getServiceManager();
    if (srvmgr)
    {
	IStaticServiceHandler *ssrvhdlr;
	ssrvhdlr = static_cast<IStaticServiceHandler *>(
	    srvmgr->getObject(XPLC_staticServiceHandler));
	if (ssrvhdlr)
	    ssrvhdlr->removeObject(oid);
    }
}


void *wvcreate(const UUID &category, WvStringParm _moniker)
{
    assert(!_moniker.isnull());

    WvString moniker(_moniker);
    moniker = trim_string(moniker.edit());

    char *cptr = strchr(moniker.edit(), ':');
    if (cptr)
	*cptr++ = 0;
    else
	cptr = "";

    DEBUGLOG("wvcreate create object ('%s' '%s').\n", moniker.cstr(), cptr);

    IServiceManager *srvmgr = XPLC_getServiceManager();
    if (!srvmgr)
	return NULL;

    ICategoryManager *catmgr;
    catmgr = static_cast<ICategoryManager *>(
	srvmgr->getObject(XPLC_categoryManager));
    if (!catmgr)
	return NULL;

    ICategory *cat = catmgr->getCategory(category);
    if (!cat)
	return NULL;

    ICategoryIterator *i = cat->getIterator();
    if (!i)
	return NULL;

    for (; !i->done(); i->next())
    {
	if (moniker == WvString(i->getString()))
	{
	    UUID oid = i->getUuid();
	    WvMonikerCreateFuncStore *func;
	    func = static_cast<WvMonikerCreateFuncStore *>(
		srvmgr->getObject(oid));
	    if (func)
		return func->create(cptr);
	}
    }
    return NULL;
}
