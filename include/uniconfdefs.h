/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines some important UniConf types.
 */
#ifndef __UNICONFDEFS_H
#define __UNICONFDEFS_H

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
    static const int NUM_DEPTHS = DESCENDENTS + 1;
        
    /**
     * Returns a (non-localized) string corresponding to a depth type.
     */
    const char *nameof(Type depth);

    /**
     * Returns a depth given its (non-localized) name, -1 if unrecognized.
     */
    Type fromname(const char *name);
};

#endif //__UNICONFDEFS_H
