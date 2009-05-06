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

#ifndef __XPLC_MODULELOADER_H__
#define __XPLC_MODULELOADER_H__

#include <xplc/utils.h>
#include <xplc/IModuleLoader.h>
#include <xplc/module.h>

class ModuleLoader: public IModuleLoader {
  IMPLEMENT_IOBJECT(ModuleLoader);
public:
  virtual IModule* loadModule(const char* modulename);
  virtual ~ModuleLoader() {}
};

class Module: public IModule {
  IMPLEMENT_IOBJECT(Module);
private:
  void *handle;
  const XPLC_ModuleInfo* moduleinfo;
public:
  Module(void* aHandle, const XPLC_ModuleInfo* aModuleInfo);
  static Module* loadModule(const char* modulename);
  virtual IObject* getObject(const UUID& cid);
  virtual ~Module();
};

#endif /* __XPLC_MODULELOADER_H__ */
