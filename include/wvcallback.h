/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVCALLBACK_H
#define __WVCALLBACK_H

#include <tr1/functional>

namespace wv
{
    /* FIXME: Needs to add autoconf test to fallback to Boost. */
    using std::tr1::function;
    using std::tr1::bind;
    using std::tr1::ref;
    using std::tr1::cref;
    using namespace std::tr1::placeholders;
}

#endif /* __WVCALLBACK_H */
