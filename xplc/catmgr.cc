/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2003, Net Integration Technologies, Inc.
 * Copyright (C) 2003, Pierre Phaneuf
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
#include <xplc/utils.h>
#include <xplc/uuidops.h>
#include "catmgr.h"
#include "category.h"

UUID_MAP_BEGIN(CategoryManager)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(ICategoryManager)
  UUID_MAP_END

CategoryManager::CategoryManager():
  categories(0) {
}

CategoryManager::~CategoryManager() {
  if(categories)
    delete categories;
}

void CategoryManager::registerComponent(const UUID& aCatid,
                                        const UUID& aUuid,
                                        const char* aString) {
  CategoryNode* cat;
  CategoryEntryNode* entry;

  for(cat = categories; cat; cat = cat->next) {
    if(cat->category == aCatid)
      break;
  }

  if(!cat) {
    cat = new CategoryNode(aCatid, categories);
    categories = cat;
  }

  assert(cat);

  for(entry = cat->entries; entry; entry = entry->next) {
    if(entry->entry == aUuid)
      return;
  }

  entry = new CategoryEntryNode(aUuid, aString, cat->entries);
  assert(entry);

  cat->entries = entry;
}

ICategory* CategoryManager::getCategory(const UUID& aUuid) {
  CategoryNode* cat;

  for(cat = categories; cat; cat = cat->next) {
    if(cat->category == aUuid)
      return new Category(this, cat->entries);
  }

  return new Category(this, NULL);
}

