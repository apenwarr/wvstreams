/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002-2003, Pierre Phaneuf
 * Copyright (C) 2002, Net Integration Technologies, Inc.
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

#include <xplc/IFactory.h>
#include <xplc/core.h>
#include <xplc/utils.h>
#include "new.h"

UUID_MAP_BEGIN(NewMoniker)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IMoniker)
  UUID_MAP_END

NewMoniker::~NewMoniker() {
}

IObject* NewMoniker::resolve(const char* aName) {
  IServiceManager* servmgr;
  IMoniker* monikers;
  IFactory* factory;
  IObject* obj = 0;

  servmgr = XPLC_getServiceManager();
  if(servmgr) {
    monikers = mutate<IMoniker>(servmgr->getObject(XPLC_monikers));

    if(monikers) {
      factory = mutate<IFactory>(monikers->resolve(aName));

      if(factory) {
        obj = factory->createObject();
        factory->release();
      }

      monikers->release();
    }

    servmgr->release();
  }

  return obj;
}

