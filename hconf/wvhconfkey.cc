/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A "key" for looking things up in a WvHConf.
 * 
 * See wvhconf.h.
 */
#include "wvhconf.h"


// null constructor: let people fill it by hand later
WvHConfKey::WvHConfKey()
{
    // leave it empty
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
WvHConfKey::WvHConfKey(const WvString &key)
{
    split(key, "/");
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
WvHConfKey::WvHConfKey(const char *key)
{
    split(key, "/");
}


// old-style 2-level key: /section/entry.
WvHConfKey::WvHConfKey(const WvString &section, const WvString &entry)
{
    append(new WvString(section), true);
    append(new WvString(entry), true);
}


// copy an old key to this key, stripping the leading components.
// This isn't a very efficient copy operation, but maybe that's okay...
WvHConfKey::WvHConfKey(const WvHConfKey &key, int offset)
{
    int count = 0;
    Iter i(key);
    
    for (count = 0, i.rewind(); count < offset && i.next(); count++)
	; // do nothing; just skipping stuff.
    if (!i.cur())
	return;
    while (i.next())
	append(new WvString(*i), true);
}


WvString WvHConfKey::printable() const
{
    return join("/");
}


