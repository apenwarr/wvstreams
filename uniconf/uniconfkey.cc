/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf hierarchical key path abstraction.
 */
#include "uniconfkey.h"
#include <climits>
#include <assert.h>

UniConfKey UniConfKey::EMPTY;
UniConfKey UniConfKey::ANY("/*"); // yes, this looks a little strange


UniConfKey::UniConfKey() : path("/")
{
}


void UniConfKey::init(WvStringParm key)
{
    assert(!key.isnull());

    // canonicalize the key
    // only make a new copy if strictly necessary
    path = key; // does not actually copy anything
    const char *src = key;
    char *dest = NULL;

    // add a slash to the beginning of the string
    if (*src != '/')
    {
        path.setsize(key.len() + 2); // must account for '/' and '\0'
        dest = path.edit();
        *(dest++) = '/';
    }
    
    // remove redundant slashes
    bool haveslash = false;
    for (char c; (c = *(src++));)
    {
        if (c != '/')
            haveslash = false;
        else
        {
            if (haveslash || *src == 0)
            {
                if (!dest)
                    dest = path.edit() + (src - key.cstr() - 1);
                continue;
            }
            haveslash = true;
        }
        if (dest)
            *dest++ = c;
    }

    // add null terminator if needed
    if (dest)
        *dest++ = 0;
}


UniConfKey::UniConfKey(const UniConfKey &other) : path(other.path)
{
}


UniConfKey::UniConfKey(const UniConfKey &_path, const UniConfKey &_key) 
    : path(_path)
{
    append(_key);
}


void UniConfKey::append(const UniConfKey &_key)
{
    if (isempty())
        path = _key.path;
    else if (!_key.isempty())
        path.append(_key.path);
}


void UniConfKey::prepend(const UniConfKey &_key)
{
    if (isempty())
        path = _key.path;
    else if (!_key.isempty())
        path = WvString("%s%s", _key.path, path);
}


bool UniConfKey::isempty() const
{
    // note: path string always has at least 1 character + null
    return path[1] == '\0';
}


int UniConfKey::numsegments() const
{
    const char *str = path + 1; // ignore leading '/'
    if (*str == '\0')
        return 0; // root has zero segments
        
    int n = 1;
    for (;;)
    {
        char c = *(str++);
        if (!c)
            break;
        if (c == '/')
            n += 1;
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
    //wverr->print("range %s-%s of \"%s\" == ", i, j, *this);
    if (i < 0)
        i = 0;
    int n = j - i;
    if (n <= 0)
        return EMPTY;

    // find beginning of range
    const char *first = path.cstr(); // points to '/' at range start
    while (i-- > 0)
    {
        first = strchr(first + 1, '/');
        if (!first)
            return EMPTY;
    }
    
    // find end of range
    int len = 1; // number of characters in range
    for (;;)
    {
        char c = first[len];
        if (!c)
        {
            if (first != path.cstr())
                break;

            // optimization: entire key requested!
            //wverr->print("\"%s\"\n", *this);
            return *this;
        }
        if (c == '/' && --n == 0)
            break;
        len += 1;
    }
    
    // construct a new key for the range
    UniConfKey result;
    result.path.setsize(len + 1);
    char *str = result.path.edit();
    memcpy(str, first, len);
    str[len] = '\0';

//    wverr->print("\"%s\"\n", result);
    return result;
}


WvString UniConfKey::printable() const
{
    return path;
}


WvString UniConfKey::strip() const
{
    // we make the string unique to avoid possible problems
    // that would occur if the UniConfKey were a temporary
    WvString result(printable() + 1);
    return result.unique();
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
