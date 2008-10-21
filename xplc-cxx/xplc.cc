/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002-2004, Net Integration Technologies, Inc.
 * Copyright (C) 2003-2004, Pierre Phaneuf
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
 *
 * As a special exception, you may use this file as part of a free
 * software library without restriction.  Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU Lesser General Public
 * License.  This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Lesser
 * General Public License.
 */

#include <xplc/IMoniker.h>
#include <xplc/IFactory.h>
#include <xplc/IModuleManagerFactory.h>
#include <xplc/xplc.h>
#include <xplc/ptr.h>

void XPLC::addModuleDirectory(const char* directory) {
  xplc_ptr<IModuleManagerFactory> factory(get<IModuleManagerFactory>(XPLC_moduleManagerFactory));

  if(!factory)
    return;

  xplc_ptr<IServiceHandler> modulemgr(factory->createModuleManager(directory));

  if(!modulemgr)
    return;

  servmgr->addHandler(modulemgr);
}

IObject* XPLC::create(const UUID& cid) {
  if(!servmgr)
    return 0;

  xplc_ptr<IFactory> factory(mutate<IFactory>(servmgr->getObject(cid)));

  if(!factory)
    return 0;

  return factory->createObject();
}

IObject* XPLC::create(const char* aMoniker) {
  if(!servmgr)
    return 0;

  xplc_ptr<IMoniker> moniker(mutate<IMoniker>(servmgr->getObject(XPLC_monikers)));
  if(!moniker)
    return 0;

  xplc_ptr<IFactory> factory(mutate<IFactory>(moniker->resolve(aMoniker)));
  if(!factory)
    return 0;

  return factory->createObject();
}


