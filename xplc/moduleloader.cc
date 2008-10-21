/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002-2004, Net Integration Technologies, Inc.
 * Copyright (C) 2002-2004, Pierre Phaneuf
 * Copyright (C) 2002-2004, Stéphane Lajoie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <assert.h>
#include <xplc/ICategoryManager.h>
#include <xplc/uuidops.h>
#include "moduleloader.h"
#include "loader.h"

UUID_MAP_BEGIN(ModuleLoader)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IModuleLoader)
  UUID_MAP_END

UUID_MAP_BEGIN(Module)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IServiceHandler)
  UUID_MAP_ENTRY(IModule)
  UUID_MAP_END

IModule* ModuleLoader::loadModule(const char* modulename) {
  return Module::loadModule(modulename);
}

Module* Module::loadModule(const char* modulename) {
  XPLC_ModuleInfo* moduleinfo = 0;
  void* dlh;
  const char* err;

  err = loaderOpen(modulename, &dlh);
  if(err)
    return NULL;

  err = loaderSymbol(dlh, "XPLC_Module",
                     reinterpret_cast<void**>(&moduleinfo));
  if(err
     || !moduleinfo
     || moduleinfo->magic != XPLC_MODULE_MAGIC) {
    loaderClose(dlh);
    return NULL;
  }

  switch(moduleinfo->version_major) {
#ifdef UNSTABLE
  case -1:
    /* nothing to do */
    break;
#endif
  default:
    loaderClose(dlh);
    return NULL;
  };

  return new Module(dlh, moduleinfo);
}

Module::Module(void* aHandle, const XPLC_ModuleInfo* aModuleInfo):
  handle(aHandle),
  moduleinfo(aModuleInfo)
{
  assert(moduleinfo);

  if(moduleinfo->categories) {
    IServiceManager* servmgr;
    IObject* obj;
    ICategoryManager* catmgr;
    const XPLC_CategoryEntry* entry;

    servmgr = XPLC_getServiceManager();
    assert(servmgr);

    obj = servmgr->getObject(XPLC_categoryManager);
    assert(obj);

    servmgr->release();

    catmgr = mutate<ICategoryManager>(obj);
    assert(catmgr);

    entry = moduleinfo->categories;
    while(entry->category != UUID_null && entry->uuid != UUID_null) {
      catmgr->registerComponent(entry->category, entry->uuid, entry->string);

      ++entry;
    }

    catmgr->release();
  }
}

IObject* Module::getObject(const UUID& cid) {
  const XPLC_ComponentEntry* entry = moduleinfo->components;
  IObject* obj = 0;

  if(!entry)
    return NULL;

  while(!obj && entry->uuid != UUID_null) {
    if(entry->uuid == cid)
      obj = entry->getObject();

    ++entry;
  }

  return obj;
}

Module::~Module() {
  /*
   * FIXME: Adding the conditional here is for future expansion, where
   * this class could be used for a non-dynamically loaded module. But
   * for some reason, the size of this file increases in an odd way
   * when it is added. This should be investigated.
   */
  if(handle)
    loaderClose(handle);
}

