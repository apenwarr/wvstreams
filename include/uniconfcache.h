/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Caching support for building UniConfGen implementations.
 */
#ifndef __UNICONFCACHE_H
#define __UNICONFCACHE_H

#include "uniconfdefs.h"
#include "uniconfkey.h"

class WvStringList;

/**
 * Some caching support for building UniConfGen implementations.
 * This API is subject to change.
 */
class UniConfCache
{
    class CacheTree;
    CacheTree *cache;

    /** Undefined. */
    UniConfCache(const UniConfCache &);

public:
    enum TriState
    {
        UNKNOWN = -1, /*!< result is unknown */
        FALSE = 0, /*!< result is negative */
        TRUE = 1 /*!< result is positive */
    };
        
    /** Create an empty cache with no known state. */
    UniConfCache();

    /** Destroy a cache and all of its contents. */
    ~UniConfCache();
    
    /** Mark node as still existing but with unknown value. */
    void mark_unknown_value(const UniConfKey &key);
    
    /** Mark node as still existing but with unknown children. */
    void mark_unknown_children(const UniConfKey &key);

    /** Mark node as having all children known. */
    void mark_known_children(const UniConfKey &key);

    /** Mark node as not having any children. */
    void mark_no_children(const UniConfKey &key);

    /** Mark node as no longer known to exist. */
    void mark_unknown_exist(const UniConfKey &key);

    /** Mark node as not existing. */
    void mark_not_exist(const UniConfKey &key);

    /** Mark node as existing.
     * Mark value as known if specified and non-null
     */
    void mark_exist(const UniConfKey &key,
        WvStringParm value = WvString::null);

    /** Recursively mark a node as changed. */
    void mark_change(const UniConfKey &key, UniConfDepth::Type depth);

    /** Determines if a node exists and returns its value in the
     * supplied string, or WvString::null if unknown. */
    TriState query_exist(const UniConfKey &key, WvString *value);

    /** Determines if a node has any children and populates the supplied
     * stringlist with their full keys if so.
     * If keys != NULL, keys will be filled in with the keys of
     * all children ONLY IF they are ALL known.
     */
    TriState query_children(const UniConfKey &key, WvStringList *keys);

private:
    /** Finds a node in the cache. */
    CacheTree *find(const UniConfKey &key);

    /** Finds a node in the cache and checks, if not found checks if
     * it could not possibly be found. */
    CacheTree *findproof(const UniConfKey &key, TriState *exists);
};

#endif //__UNICONFCACHE_H
