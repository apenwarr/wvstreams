/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2004, Net Integration Technologies, Inc.
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
#include "catiter.h"

UUID_MAP_BEGIN(CategoryIterator)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(ICategoryIterator)
  UUID_MAP_END

CategoryIterator::CategoryIterator(ICategory* aCategory,
                                   CategoryEntryNode* aEntries):
  category(aCategory),
  current(aEntries) {
  /*
   * Prevent the category from dying, which in turn prevents the
   * category manager from dying (which would free the list).
   */
  category->addRef();
}

const UUID& CategoryIterator::getUuid() {
  if(current)
    return current->entry;

  return UUID_null;
}

const char* CategoryIterator::getString() {
  if(current)
    return current->str;

  return 0;
}

void CategoryIterator::next() {
  if(current)
    current = current->next;
}

bool CategoryIterator::done() {
  return current == 0;
}

CategoryIterator::~CategoryIterator() {
  category->release();
}

