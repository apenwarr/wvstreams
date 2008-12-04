/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002-2004, Net Integration Technologies, Inc.
 * Copyright (C) 2002-2004, Pierre Phaneuf
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
#include "modulemgr.h"
#include <xplc/IModuleLoader.h>

#include "config.h"

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#if !defined(WIN32)
# if HAVE_DIRENT_H
#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
# else
#  define dirent direct
#  define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#   include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#   include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#   include <ndir.h>
#  endif
# endif
#else
# include <io.h>
#endif

#include <stdio.h>

UUID_MAP_BEGIN(ModuleManagerFactory)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IModuleManagerFactory)
  UUID_MAP_END

UUID_MAP_BEGIN(ModuleManager)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IServiceHandler)
  UUID_MAP_END

struct ModuleNode {
  ModuleNode* next;
  IModule* module;
  ModuleNode(IModule* aModule, ModuleNode* aNext):
    next(aNext), module(aModule) {
    assert(module);
  }
  ~ModuleNode() {
    if(module)
      module->release();
  }
};

#ifdef SOLARIS
#define PATH_MAX 4096
#endif

IServiceHandler* ModuleManagerFactory::createModuleManager(const char* directory) {
#if !defined(WIN32)
  DIR* dir;
  struct dirent* ent;
  char fname[PATH_MAX];
  IServiceManager* servmgr = XPLC_getServiceManager();
  IModuleLoader* loader;
  ModuleNode* modules = 0;

  if(!servmgr)
    return 0;

  loader = mutate<IModuleLoader>(servmgr->getObject(XPLC_moduleLoader));
  servmgr->release();
  if(!loader)
    return 0;

  dir = opendir(directory);
  if(!dir) {
    loader->release();
    return 0;
  }

  rewinddir(dir);
  while((ent = readdir(dir))) {
    IModule* module;

    snprintf(fname, PATH_MAX, "%s/%s", directory, ent->d_name);

    module = loader->loadModule(fname);
    if(module) {
      ModuleNode* node = new ModuleNode(module, modules);

      if(node)
        modules = node;
    }
  }

  loader->release();

  closedir(dir);

  return new ModuleManager(modules);

#else

  intptr_t dir;
  _finddata_t ent;
  char fname[4096];
  char pattern[4096];
  IServiceManager* servmgr = XPLC_getServiceManager();
  IModuleLoader* loader;
  ModuleNode* modules = 0;

  if(!servmgr)
    return 0;

  loader = mutate<IModuleLoader>(servmgr->getObject(XPLC_moduleLoader));
  servmgr->release();
  if(!loader)
    return 0;

  snprintf(pattern, sizeof(pattern), "%s/*.*", directory);

  dir = _findfirst(pattern, &ent);

  if(!dir) {
    loader->release();
    return 0;
  }

  do {
    IModule* module;

    _snprintf(fname, sizeof(fname), "%s/%s", directory, ent.name);

    module = loader->loadModule(fname);
    if(module) {
      ModuleNode* node = new ModuleNode(module, modules);

      if(node)
        modules = node;
    }
  } while(_findnext(dir, &ent) == 0);

  loader->release();

  _findclose(dir);

  return new ModuleManager(modules);
#endif
}

ModuleManager::ModuleManager(ModuleNode* aModules):
  modules(aModules) {
}

IObject* ModuleManager::getObject(const UUID& cid) {
  ModuleNode* node = modules;

  while(node) {
    IObject* obj = node->module->getObject(cid);

    if(obj)
      return obj;

    node = node->next;
  }

  return 0;
}

ModuleManager::~ModuleManager() {
  ModuleNode* node = modules;

  while(node) {
    ModuleNode* next = node->next;

    delete node;

    node = next;
  }
}

