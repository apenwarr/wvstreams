/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * Implementation of an efficient lookup for a set characters.
 *
 * It is, however, a little space intensive, but you should statically
 * create them in your functions, and then they won't be so bad.
 */
#include "wvstringmask.h"

WvStringMask::WvStringMask(WvStringParm s)
{
    zap();
    set(s, true);
}

WvStringMask::WvStringMask(char c)
{
    zap();
    set(c, true);
}

bool WvStringMask::operator[](const char c) const
{
    unsigned char uc = c;
    return _set[uc];
}

const char WvStringMask::first() const
{
    return _first;
}

void WvStringMask::zap()
{
    memset(_set, 0, sizeof(bool) * sizeof(_set));
    _first = '\0';
}

void WvStringMask::set(const char c, bool value)
{
    if (!_first)
	_first = c;

    _set[unsigned(c)] = value;
}

void WvStringMask::set(WvStringParm s, bool value)
{
    if (!s.isnull())
    {
	const char *c = s.cstr();

	if (!_first)
	    _first = *c;

	while (*c)
	{
	    _set[unsigned(*c)] = value;
	    ++c;
	}
    }
}
