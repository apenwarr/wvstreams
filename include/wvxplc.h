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

#if 0
#include <xplc/IObject.h>
#define deletev delete[]
#else
#include <xplc/delete.h>
#endif

#include <xplc/xplc.h>

#include "wvtypetraits.h"

#define RELEASE(ptr) do { WvTraits<typeof(*ptr)>::release(ptr); ptr = 0; } while (0)

#endif // __WVXPLC_H
