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

#ifndef __XPLC_CATITER_H__
#define __XPLC_CATEITER_H__

#include <xplc/ICategory.h>
#include <xplc/ICategoryIterator.h>
#include "categorynode.h"

class CategoryIterator: public ICategoryIterator {
  IMPLEMENT_IOBJECT(CategoryIterator);
private:
  ICategory* category;
  CategoryEntryNode* current;
public:
  CategoryIterator(ICategory*, CategoryEntryNode*);
  virtual const UUID& getUuid();
  virtual const char* getString();
  virtual void next();
  virtual bool done();
  virtual ~CategoryIterator();
};

#endif /* __XPLC_CATEITER_H__ */
