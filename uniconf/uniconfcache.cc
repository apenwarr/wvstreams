/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Caching support for building UniConfGen implementations.
 */
#include "uniconfcache.h"
#include "uniconftree.h"
#include "wvstringlist.h"

/***** UniConfCache::CacheTree *****/

/**
 * @internal
 * Cache tree structure.
 */
class UniConfCache::CacheTree : public UniConfTree<CacheTree>
{
public:
    WvString xvalue; /*!< the value of this entry, WvString::null if unknown */
    bool xchildren; /*!< true if the list of children is known */
    
    CacheTree(CacheTree *parent, const UniConfKey &key,
        WvStringParm value = WvString::null) :
        UniConfTree<CacheTree>(parent, key),
        xvalue(value), xchildren(false)
    {
    }
};



/***** UniConfCache *****/

UniConfCache::UniConfCache() :
    cache(NULL)
{
}


UniConfCache::~UniConfCache()
{
    delete cache;
}

void UniConfCache::mark_unknown_value(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;
    
    node->xvalue = WvString::null;
}


void UniConfCache::mark_unknown_children(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;
    
    node->xchildren = false;
    node->zap();
}


void UniConfCache::mark_known_children(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;
    
    node->xchildren = true;
}


void UniConfCache::mark_no_children(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;
    
    node->xchildren = true;
    node->zap();
}


void UniConfCache::mark_unknown_exist(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;
    
    if (node == cache)
        cache = NULL;
    else
        node->parent()->xchildren = false;
    delete node;
}


void UniConfCache::mark_not_exist(const UniConfKey &key)
{
    CacheTree *node = find(key);
    if (! node) return;

    if (node == cache)
        cache = NULL;
    delete node;
}


void UniConfCache::mark_exist(const UniConfKey &key, WvStringParm value)
{
    if (! cache)
        cache = new CacheTree(NULL, UniConfKey::EMPTY);
    CacheTree *node = cache;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        CacheTree *prev = node;
        node = prev->findchild(it());
        if (! node)
            node = new CacheTree(prev, it());
    }

    if (! value.isnull())
        node->xvalue = value;
}


void UniConfCache::mark_change(const UniConfKey &key,
    UniConfDepth::Type depth)
{
    switch (depth)
    {
        case UniConfDepth::ZERO:
        case UniConfDepth::ONE:
        case UniConfDepth::INFINITE:
            mark_unknown_exist(key);
            break;
        case UniConfDepth::CHILDREN:
        case UniConfDepth::DESCENDENTS:
            mark_unknown_children(key);
            break;
    }
}


UniConfCache::TriState UniConfCache::query_exist(const UniConfKey &key,
    WvString *value)
{
    TriState result;
    CacheTree *node = findproof(key, & result);
    if (value)
        *value = node ? node->xvalue : WvString();
    return result;
}

/** Determines if a node has any children and populates the supplied
 * stringlist with their full keys if so.
 * If keys != NULL, keys will be filled in with the keys of
 * all children ONLY IF they are ALL known.
 */
UniConfCache::TriState UniConfCache::query_children(const UniConfKey &key,
    WvStringList *keys)
{
    TriState result;
    CacheTree *node = findproof(key, & result);
    if (! node)
        return result;
    if (node->xchildren)
    {
        if (! node->haschildren())
            return FALSE;
        if (keys)
        {
            CacheTree::Iter it(*node);
            for (it.rewind(); it.next(); )
                keys->append(new WvString(it->key()), true);
        }
    }
    else if (! node->haschildren())
        return UNKNOWN;
    return TRUE;
}


UniConfCache::CacheTree *UniConfCache::find(const UniConfKey &key)
{
    if (! cache)
        return NULL;
    return cache->find(key);
}



UniConfCache::CacheTree *UniConfCache::findproof(const UniConfKey &key,
    TriState *exists)
{
    *exists = UNKNOWN;
    if (! cache)
        return NULL;
    CacheTree *node = cache;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        CacheTree *prev = node;
        node = prev->findchild(it());
        if (! node)
        {
            // check if parent is sure we don't exist
            if (prev->xchildren)
                *exists = FALSE;
            return NULL;
        }
    }
    *exists = TRUE;
    return node;
}
