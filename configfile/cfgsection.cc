/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Implementation of the WvConfigSection class. 
 *
 * Created:     Sept 28 1997            D. Coombs
 *
 */
#include "wvconf.h"


WvConfigSection::WvConfigSection(WvStringParm _name)
	: name(_name)
{
}


WvConfigSection::~WvConfigSection()
{
    // the WvConfigEntryList destructor automatically deletes all its
    // entries, so no need to worry about doing that.
}


WvConfigEntry *WvConfigSection::operator[] (WvStringParm ename)
{
    Iter i(*this);

    for (i.rewind(); i.next();)
    {
	if (strcasecmp(i().name, ename) == 0)
	    return &i();
    }

    return NULL;
}


const char *WvConfigSection::get(WvStringParm entry, const char *def_val)
{
    WvConfigEntry *e = (*this)[entry];
    return e ? (const char *)e->value : def_val;
}


void WvConfigSection::set(WvStringParm entry, WvStringParm value)
{
    WvConfigEntry *e = (*this)[entry];
    
    // need to delete the entry?
    if (!value || !value[0])
    {
	if (e) unlink(e);
	return;
    }

    // otherwise, add the entry requested
    if (e)
	e->set(value);
    else
	append(new WvConfigEntry(entry, value), true);
}


void WvConfigSection::quick_set(WvStringParm entry, WvStringParm value)
{
    append(new WvConfigEntry(entry, value), true);
}


void WvConfigSection::dump(WvStream &fp)
{
    Iter i(*this);

    for (i.rewind(); i.next(); )
    {
	WvConfigEntry &e = *i;
	if (e.value && e.value[0])
	    fp.print("%s = %s\n", e.name, e.value);
	else
	    fp.print("%s =\n", e.name);
    }
}
