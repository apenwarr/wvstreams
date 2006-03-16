/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf hierarchical key path abstraction.
 */
#include "wvassert.h"
#include "wvstream.h"
#include "uniconfkey.h"
#include "wvhash.h"
#include <climits>
#include <assert.h>
#include <strutils.h>

unsigned WvHash(const UniConfKey &k)
{
    int numsegs = k.right - k.left;
    unsigned result;
    switch (numsegs)
    {
        case 0:
            result = 0;
            break;
        case 1:
            result = WvHash(k.store->segments[k.left]);
            break;
        default:
            result = WvHash(k.store->segments[k.left])
                ^ WvHash(k.store->segments[k.right - 1])
                ^ numsegs;
            break;
    }
    return result;
}

// The initial value of 1 for the ref_count of these guarantees
// that they won't ever be deleted
UniConfKey::Store UniConfKey::EMPTY_store(1, 1);
UniConfKey::Store UniConfKey::ANY_store(1, 1, "*");
UniConfKey::Store UniConfKey::RECURSIVE_ANY_store(1, 1, "...");

UniConfKey UniConfKey::EMPTY(&EMPTY_store, 0, 0);
UniConfKey UniConfKey::ANY(&ANY_store, 0, 1);
UniConfKey UniConfKey::RECURSIVE_ANY(&RECURSIVE_ANY_store, 0, 1);


UniConfKey::Store::Store(int size, int _ref_count,
        WvStringParm key) :
    segments(size),
    ref_count(_ref_count)
{
    if (!key)
        return;

    WvStringList parts;
    parts.split(key, "/");

    segments.resize(parts.count());
    WvStringList::Iter part(parts);
    for (part.rewind(); part.next(); )
    {
        if (!*part)
            continue;
        segments.append(*part);
    }
    if (!!key && key[key.len()-1] == '/' && segments.used() > 0)
        segments.append(Segment());
}


UniConfKey &UniConfKey::collapse()
{
    if ((right - left == 1 && !store->segments[right-1])
        || right == left)
    {
        if (--store->ref_count == 0)
            delete store;
        store = &EMPTY_store;
        left = right = 0;
        ++store->ref_count;
    }
    return *this;
}
 

void UniConfKey::unique()
{
    if (store->ref_count == 1)
        return;
    store->ref_count--;
    Store *old_store = store;
    store = new Store(right - left, 1);
    for (int i=left; i<right; ++i)
        store->segments.append(old_store->segments[i]);
    right -= left;
    left = 0;
}
    
UniConfKey::UniConfKey(const UniConfKey &_path, const UniConfKey &_key) :
    store(new Store(_path.numsegments() + _key.numsegments() + 1, 1)),
    left(0),
    right(0)
{
    bool hastrailingslash = _key.isempty() || _key.hastrailingslash();
    for (int i=_path.left; i<_path.right; ++i)
    {
        const Segment &segment = _path.store->segments[i];
        if (!segment)
            continue;
        store->segments.append(segment);
        ++right;
    }
    for (int j=_key.left; j<_key.right; ++j)
    {
        const Segment &segment = _key.store->segments[j];
        if (!segment)
            continue;
        store->segments.append(segment);
        ++right;
    }
    if (hastrailingslash)
    {
        store->segments.append("");
        ++right;
    }
    collapse();
}


void UniConfKey::append(const UniConfKey &_key)
{
    bool hastrailingslash = _key.isempty() || _key.hastrailingslash();
    unique();
    store->segments.resize(right - left + _key.right - _key.left + 1);
    for (int j=_key.left; j<_key.right; ++j)
    {
        const Segment &segment = _key.store->segments[j];
        if (!segment)
            continue;
        store->segments.replace(right, segment);
        ++right;
    }
    if (hastrailingslash)
    {
        store->segments.replace(right, "");
        ++right;
    }
    collapse();
}


void UniConfKey::prepend(const UniConfKey &_key)
{
    unique();
    int shift = 0;
    for (int j=_key.left; j<_key.right; ++j)
    {
        if (!!_key.store->segments[j])
            ++shift;
    }
    store->segments.resize(shift + right - left, shift);
    for (int j=_key.left; j<_key.right; ++j)
    {
        const Segment &segment = _key.store->segments[j];
        if (!segment)
            continue;
        store->segments.replace(left + j - _key.left, segment);
        ++right;
    }
    collapse();
}


bool UniConfKey::iswild() const
{
    for (int i=left; i<right; ++i)
        if (store->segments[i].iswild())
            return true;
    return false;
}


UniConfKey UniConfKey::pop(int n)
{
    if (n == 0)
        return UniConfKey();
    unique();
    if (n > right - left)
        n = right - left;
    if (n < 0)
        n = 0;
    int old_left = left;
    left += n;
    UniConfKey result(store, old_left, left);
    collapse();
    return result.collapse();
}


UniConfKey UniConfKey::range(int i, int j) const
{
    if (j > right - left)
        j = right - left;
    if (i < 0)
        i = 0;
    if (j < i)
        j = i;
    return UniConfKey(store, left + i, left + j).collapse();
}


WvString UniConfKey::printable() const
{
    switch (right - left)
    {
        case 0:
            return WvString::empty;
        case 1:
            return store->segments[left];
        default:
        {
            WvDynBuf buf;
            for (int i=left; i<right; ++i)
            {
                buf.putstr(store->segments[i]);
                if (i < right-1)
                    buf.put('/');
            }
            return buf.getstr();
        }
    }
}


int UniConfKey::compareto(const UniConfKey &other) const
{
    int i, j;
    for (i=left, j=other.left; i<right && j<other.right; ++i, ++j)
    {
        int val = strcasecmp(store->segments[i], other.store->segments[j]);
        if (val != 0)
            return val;
    }
    if (i == right)
    {
        if (j == other.right)
            return 0;
        else
            return -1;
    }
    else
        return 1;
}


bool UniConfKey::matches(const UniConfKey &pattern) const
{
    // TODO: optimize this function
    if (*this == pattern)
        return true;
    
    UniConfKey head(pattern.first());

    // handle * wildcard
    if (head == UniConfKey::ANY)
    {
        if (isempty())
            return false;
        return removefirst().matches(pattern.removefirst());
    }

    // handle ... wildcard
    if (head == UniConfKey::RECURSIVE_ANY)
    {
        UniConfKey tail(pattern.removefirst());
        if (tail.isempty())
            return true; // recursively matches anything
        for (int n = 0; ; ++n)
        {
            UniConfKey part(removefirst(n));
            if (part.matches(tail))
                return true;
            if (part.isempty())
                break;
        }
        return false;
    }
    
    // no other wildcard arrangements currently supported
    return false;
}


bool UniConfKey::suborsame(const UniConfKey &key) const
{
    int n = numsegments();
    if (hastrailingslash())
	n -= 1;

    if (key.first(n) == first(n))
	return true;
    return false;
}


bool UniConfKey::suborsame(const UniConfKey &key, UniConfKey &subkey) const
{
    int n = numsegments();

    // Compensate for the directory-style naming convention of the
    // trailing slash.
    if (hastrailingslash())
	n -= 1;

    if (key.first(n) == first(n))
    {
	subkey = key.removefirst(n);
	return true;
    }
    return false;
}


UniConfKey UniConfKey::subkey(const UniConfKey &key) const
{
    UniConfKey answer;
    wvassert(suborsame(key, answer),
	     "this = '%s'\nkey = '%s'", printable(), key);
    return answer;
}
