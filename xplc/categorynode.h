/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2000, Pierre Phaneuf
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

#ifndef __XPLC_CATEGORYNODE_H__
#define __XPLC_CATEGORYNODE_H__

#include <string.h>
#include <stdlib.h>

class CategoryEntryNode {
public:
  CategoryEntryNode* next;
  UUID entry;
  char* str;
  CategoryEntryNode(const UUID& aUuid,
                    const char* aStr,
                    CategoryEntryNode* aNext): next(aNext),
                                               entry(aUuid),
                                               str(aStr ? strdup(aStr) : 0) {
  }
  ~CategoryEntryNode() {
    if(next)
      delete next;

    if(str)
      free(str);
  }
};

class CategoryNode {
public:
  CategoryNode* next;
  UUID category;
  CategoryEntryNode* entries;
  CategoryNode(const UUID& aUuid,
               CategoryNode* aNext): next(aNext),
                                     category(aUuid),
                                     entries(0) {
  }
  ~CategoryNode() {
    if(entries)
      delete entries;

    if(next)
      delete next;
  }
};

#endif /* __XPLC_CATEGORYNODE_H__ */
