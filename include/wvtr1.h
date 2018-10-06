/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provide some aliases for TR1 stuff.
 * FIXME: totally obsolete. time to update to c++11.
 */
#ifndef __WVTR1_H
#define __WVTR1_H

#include "wvautoconf.h"

#if defined(HAVE_TR1_FUNCTIONAL)

#include <tr1/functional>
#include <tr1/memory>

namespace wv
{
    using std::tr1::bind;
    using std::tr1::cref;
    using std::tr1::function;
    using std::tr1::ref;
    using std::tr1::shared_ptr;
}

namespace {
    using namespace std::tr1::placeholders;
}

#else
#error "TR1 is required to use WvStreams"
#endif

#endif /* __WVTR1_H */
