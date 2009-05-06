/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * XPLC - Cross-Platform Lightweight Components
 * Copyright (C) 2000-2002, Pierre Phaneuf
 * Copyright (C) 2001, Stéphane Lajoie
 * Copyright (C) 2002-2004, Net Integration Technologies, Inc.
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

#ifndef __XPLC_SERVMGR_H__
#define __XPLC_SERVMGR_H__

#include <xplc/IServiceManager.h>
#include "handlernode.h"

class ServiceManager: public IServiceManager {
  IMPLEMENT_IOBJECT(ServiceManager);
private:
  HandlerNode* handlers;
public:
  ServiceManager():
    handlers(0) {
  }
  virtual ~ServiceManager();
  /* IServiceManager */
  virtual void addHandler(IServiceHandler*);
  virtual void addFirstHandler(IServiceHandler*);
  virtual void addLastHandler(IServiceHandler*);
  virtual void removeHandler(IServiceHandler*);
  virtual IObject* getObject(const UUID&);
};

#endif /* __XPLC_SERVMGR_H__ */
