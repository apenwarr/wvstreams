/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * XPLC setup bits.
 */ 
#ifndef __WVXPLC_H
#define __WVXPLC_H

#ifndef UNSTABLE
#define UNSTABLE
#endif

#include "wvautoconf.h"

#ifndef ENABLE_DELETE_DETECTOR
#include <xplc/IObject.h>
#define deletev delete[]
#else
#include <xplc/delete.h>
#endif

#include <xplc/xplc.h>
#include <xplc/ptr.h>
#include <xplc/uuidops.h>

/*
 * There is another definition of DELETE in <arpa/nameser_compat.h>,
 * but we don't care about it.
 */
#undef DELETE
#define RELEASE(ptr) do { if (ptr) ptr->release(); ptr = 0; } while (0)
#define DELETE(ptr) do { delete ptr; ptr = 0; } while (0)

#endif // __WVXPLC_H
