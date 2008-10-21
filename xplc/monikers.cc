/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2002, Net Integration Technologies, Inc.
 * Copyright (C) 2002-2003, Pierre Phaneuf
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

#include <xplc/core.h>
#include <xplc/utils.h>
#include "monikers.h"

#define MONIKER_SEPARATOR_CHAR ':'

UUID_MAP_BEGIN(MonikerService)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IMoniker)
  UUID_MAP_ENTRY(IMonikerService)
  UUID_MAP_END

MonikerService::~MonikerService() {
  MonikerNode* node;
  MonikerNode* ptr;

  node = monikers;

  while(node) {
    ptr = node;
    node = node->next;
    delete ptr;
  }

  monikers = 0;
}

IObject* MonikerService::resolve(const char* aName) {
  MonikerNode* node;
  IServiceManager* servmgr;
  IObject* obj = 0;
  IMoniker* moniker;
  char* name = strdup(aName);
  char* rest = strchr(name, MONIKER_SEPARATOR_CHAR);

  node = monikers;

  if(rest) {
    *rest = 0;
    ++rest;
  }

  while(node) {
    if(strcmp(name, node->name) == 0) {
      servmgr = XPLC_getServiceManager();
      if(!servmgr)
        break;

      obj = servmgr->getObject(node->uuid);
      servmgr->release();

      if(rest) {
        moniker = mutate<IMoniker>(obj);
        if(moniker) {
          obj = moniker->resolve(rest);
          moniker->release();
        } else
          obj = 0;
      }

      break;
    }

    node = node->next;
  }

  free(name);

  return obj;
}

void MonikerService::registerObject(const char* aName, const UUID& aUuid) {
  MonikerNode* node;

  /*
   * FIXME: we should do something about registering a name that
   * contains the separator character.
   */

  node = monikers;

  while(node) {
    if(strcmp(aName, node->name) == 0)
      break;

    node = node->next;
  }

  /*
   * FIXME: maybe add a "replace" bool parameter? Or would this
   * encourage moniker hijacking too much?
   */
  if(node)
    return;

  node = new MonikerNode(aName, aUuid, monikers);
  monikers = node;
}

