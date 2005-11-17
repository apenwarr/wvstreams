/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf hierarchical key path abstraction.
 */
#include "wvassert.h"
#include "wvstream.h"
#include "uniconfkey.h"
#include <climits>
#include <assert.h>
#include <strutils.h>

UniConfKey UniConfKey::EMPTY;
UniConfKey UniConfKey::ANY("*");
UniConfKey UniConfKey::RECURSIVE_ANY("...");


UniConfKey::UniConfKey() :
    path("") // important to ensure we don't get nil paths everywhere
             // since for the moment we are not equipped to deal with them
{
}


void UniConfKey::init(WvStringParm key)
{
    assert(!key.isnull());

    // canonicalize the key by removing leading/trailing slashes, and
    // changing multiple slashes in a row to single slashes.

    if (key[0] == '/' || *(strchr(key, 0)-1) == '/' || strstr(key, "//"))
    {
	path.setsize(strlen(key) + 1);
	char *optr = path.edit();
	const char *iptr = key + int(strspn(key, "/"));
	
	while (*iptr)
	{
	    if (*iptr == '/')
	    {
		// if there's more than one slash, this finds the last one:
		iptr += strspn(iptr, "/") - 1;
		
		// if there's nothing after the slash, it's a terminating
		// slash.  copy the terminating slash and then stop.
		if (!iptr[1])
		{
		    *optr++ = *iptr++;
		    break;
		}
		
		// if we get here, it's exactly one intermediate slash.
	    }
	    
	    *optr++ = *iptr++;
	}
	
	*optr = 0;
    }
    else // easy: already in good shape!  Use WvString's optimized copying.
	path = key;
}


UniConfKey::UniConfKey(const UniConfKey &other) : path(other.path)
{
}


UniConfKey::UniConfKey(const UniConfKey &_path, const UniConfKey &_key) 
{
    if (!_path.path)
	path = _key;
    else
	path = spacecat(_path, _key, '/', true);
}


void UniConfKey::append(const UniConfKey &_key)
{
    if (!path)
        path = _key.path;
    else
	path = spacecat(path, _key.path, '/', true);
}


void UniConfKey::prepend(const UniConfKey &_key)
{
    if (!path)
        path = _key.path;
    else if (!!_key.path)
        path = spacecat(_key.path, path, '/', true);
}


bool UniConfKey::isempty() const
{
    return !path;
}


bool UniConfKey::iswild() const
{
    // FIXME: not precise
    return strchr(path, '*') || strstr(path, "...");
}


bool UniConfKey::hastrailingslash() const
{
    const char *s = path.cstr();
    return s[strlen(s) - 1] == '/';
}


int UniConfKey::numsegments() const
{
    if (!path)
	return 0;
    
    int n = 1; // all non-null paths have at least one segment
    for (const char *cptr = path; *cptr; cptr++)
    {
        if (*cptr == '/')
            n++;
    }
    return n;
}


UniConfKey UniConfKey::segment(int n) const
{
    return range(n, n + 1);
}


UniConfKey UniConfKey::pop(int n)
{
    UniConfKey res = range(0,n);
    *this = range(n, INT_MAX);
    return res;
}


UniConfKey UniConfKey::first(int n) const
{
    return range(0, n);
}


UniConfKey UniConfKey::last(int n) const
{
    return range(numsegments() - n, INT_MAX);
}


UniConfKey UniConfKey::removefirst(int n) const
{
    return range(n, INT_MAX);
}


UniConfKey UniConfKey::removelast(int n) const
{
    return range(0, numsegments() - n);
}


UniConfKey UniConfKey::range(int i, int j) const
{
    if (!path) return *this;
    
    const char *sptr, *eptr;
    int count;
    
    // find the beginning
    for (sptr = path, count = 0; *sptr && count < i; sptr++)
    {
	if (*sptr == '/')
	    count++;
    }
    
    // find the end
    for (eptr = sptr; *eptr; eptr++)
    {
	if (*eptr == '/')
	    count++;
	
	if (count >= j)
	    break; // don't want to increment eptr
    }
    
    // optimization: they got the whole key!  Don't copy.
    if (sptr == path && !*eptr)
	return *this;

    // otherwise, return a new key.
    UniConfKey result; // avoid running the normalizing constructor
    int len = eptr - sptr;
    if (len)
    {
        if (*eptr)
        {
            result.path.setsize(len + 1);
            char *cptr = result.path.edit();
            strncpy(cptr, sptr, len);
            cptr[len] = 0;
        }
        else
        {
            // optimization: they got the end of the key, so we can share the
            // buffer and avoid a copy.
            result.path = path.offset(sptr - path.cstr());
        }
    }
    return result;
}


WvString UniConfKey::printable() const
{
    return path;
}


UniConfKey &UniConfKey::operator= (const UniConfKey &other)
{
    path = other.path;
    return *this;
}


int UniConfKey::compareto(const UniConfKey &other) const
{
    return strcasecmp(path, other.path);
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

    UniConfKey k = key.first(numsegments());

    if (key.first(n) == first(n))
	return true;
    return false;
}


bool UniConfKey::suborsame(const UniConfKey &key, WvString &subkey) const
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
    WvString answer;
    wvassert(suborsame(key, answer),
	     "this = '%s'\nkey = '%s'", printable(), key);
    return answer;
}
