/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A "key" for looking things up in a UniConf.
 * 
 * See uniconf.h.
 */
#include "uniconf.h"


// null constructor: let people fill it by hand later
UniConfKey::UniConfKey()
{
    // leave it empty
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
UniConfKey::UniConfKey(WvStringParm key)
{
    split(key, "/");
    if (count() == 1 && !*first())
	zap();
}


// string-style hierarchical key (separated by '/' characters)
// ...maybe I'll extend this later to support old-style [section]entry syntax.
UniConfKey::UniConfKey(const char *key)
{
    split(key, "/");
    if (count() == 1 && !*first())
	zap();
}


// old-style 2-level key: /section/entry.
UniConfKey::UniConfKey(WvStringParm section, WvStringParm entry)
{
    append(new WvString(section), true);
    append(new WvString(entry), true);
}


// copy an old key to this key, stripping the leading components.
// This isn't a very efficient copy operation, but maybe that's okay...
UniConfKey::UniConfKey(const UniConfKey &key, int offset, int max)
{
    int count = 0;
    Iter i(key);
    
    for (count = 0, i.rewind(); count < offset && i.next(); count++)
	; // do nothing; just skipping stuff.
    if (!i.cur())
	return;
    
    // note: if 'max' is negative, this loop won't terminate because of
    // max.  (ie. max will never reach zero).  That's the right behaviour!
    while (i.next() && max--)
	append(new WvString(*i), true);
}


UniConfString UniConfKey::printable() const
{
    if (isempty() || (count()==1 && !*first()))
	return "/";
    else
	return join("/");
}


