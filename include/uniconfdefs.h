/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Defines some important UniConf types.
 */
#ifndef __UNICONFDEFS_H
#define __UNICONFDEFS_H

#include "wvcallback.h"

class UniConf;

namespace UniConfDepth
{
    /**
     * An enumeration that represents the traversal depth for recursive
     * operations.
     */
    enum Type
    {
        ZERO = 0, /*!< considers this key only */
        ONE = 1,  /*!< considers this key and all direct children */
        INFINITE = 2, /*!< considers this key and all descendents */
        CHILDREN = 3, /*!< considers all direct children */
        DESCENDENTS = 4 /*!< considers all descendents */
    };
};

// parameters are: UniConf object, userdata
DeclareWvCallback(2, void, UniConfCallback, const UniConf &, void *);

#endif //__UNICONFDEFS_H
