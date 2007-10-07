/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provide some aliases for TR1 stuff, with a fallback on Boost.
 */
#ifndef __WVTR1_H
#define __WVTR1_H

#include "wvautoconf.h"

#if defined(HAVE_TR1_FUNCTIONAL)

#include <tr1/functional>

namespace wv
{
    using std::tr1::bind;
    using std::tr1::cref;
    using std::tr1::function;
    using std::tr1::ref;
}

namespace {
    using namespace std::tr1::placeholders;
}

#elif defined(HAVE_BOOST_FUNCTION_HPP)

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace wv
{
    using boost::bind;
    using boost::cref;
    using boost::function;
    using boost::ref;
}

#else /* We have neither TR1 or Boost, punt. */
#error "One of TR1 or Boost is required to use WvStreams"
#endif

#endif /* __WVTR1_H */
