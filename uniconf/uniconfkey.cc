/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf hierarchical key path abstraction.
 */
#include "wvstream.h"
#include "uniconfkey.h"
#include <climits>
#include <assert.h>

UniConfKey UniConfKey::EMPTY;
UniConfKey UniConfKey::ANY("*"); // yes, this looks a little strange


UniConfKey::UniConfKey()
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
	const char *iptr = key + strspn(key, "/");
	
	while (*iptr)
	{
	    if (*iptr == '/')
	    {
		// if there's more than one slash, this finds the last one:
		iptr += strspn(iptr, "/") - 1;
		
		// if there's nothing after the slash, it's a terminating
		// slash; stop now.
		if (!iptr[1]) break;
		
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
    else if (!_key.path)
	path = _path;
    else
	path = WvString("%s/%s", _path, _key.path);
}


void UniConfKey::append(const UniConfKey &_key)
{
    if (!path)
        path = _key.path;
    else if (!!_key.path)
	path = WvString("%s/%s", path, _key.path);
}


void UniConfKey::prepend(const UniConfKey &_key)
{
    if (!path)
        path = _key.path;
    else if (!!_key.path)
        path = WvString("%s/%s", _key.path, path);
}


bool UniConfKey::isempty() const
{
    return !path;
}


bool UniConfKey::iswild() const
{
    return strchr(path, '*') != NULL;
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
    WvString s;
    s.setsize(eptr-sptr+1);
    char *cptr = s.edit();
    strncpy(cptr, sptr, eptr-sptr);
    cptr[eptr-sptr] = 0;
    
    return s;
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


bool UniConfKey::matches(const UniConfKey &other) const
{
    // could be optimized a bit
    if (*this == other)
        return true;
    if (other.first() == UniConfKey::ANY)
        return removefirst().matches(other.removefirst());
    // no other wildcard arrangements currently supported
    return false;
}
