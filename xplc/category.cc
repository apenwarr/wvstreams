/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2003, Net Integration Technologies, Inc.
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

#include <xplc/utils.h>
#include "category.h"
#include "catiter.h"

UUID_MAP_BEGIN(Category)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(ICategory)
  UUID_MAP_END

Category::Category(ICategoryManager* aMgr, CategoryEntryNode* aEntries):
  mgr(aMgr),
  entries(aEntries) {
  /*
   * Prevent the category manager from dying (which would free the
   * list).
   */
  mgr->addRef();
}

ICategoryIterator* Category::getIterator() {
  return new CategoryIterator(this, entries);
}

Category::~Category() {
  mgr->release();
}

