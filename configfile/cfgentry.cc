/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Implementation of the WvConfigEntry class. 
 *
 * Created:     Sept 28 1997            D. Coombs
 *
 */
#include "wvconf.h"

WvConfigEntry::WvConfigEntry()
{
}

WvConfigEntry::WvConfigEntry(const WvString &_name, const WvString &_value)
	: name(_name), value(_value)
{
    name.unique();
    value.unique();
}

WvConfigEntry::~WvConfigEntry()
{
}
