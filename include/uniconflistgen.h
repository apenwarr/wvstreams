/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfListGen is a UniConf generator to allow multiple generators to be
 * stacked in a priority sequence for get/set/etc.
 *
 */

#ifndef __UNICONFLISTGEN_H
#define __UNICONFLISTGEN_H

#include "uniconfgen.h"

/*
 * Accepts a list of UniConf generators, and stacks them, treating them as one
 * uniconf source.
 *
 * The standard way of using the list generator would be with a moniker. The
 * moniker takes the form of list:(tcl style string list).
 *
 * For example: list:readonly:ini:admin.ini ini:user.ini
 *
 * The list can also contain a list. This still uses tcl style string lists as
 * follows:
 *
 * list:readonly:ini:admin.ini list:{ini:user1.ini ini:user2.ini} ini:def.ini
 */
class UniConfListGen : public UniConfGen
{
public:
    UniConfListGen(UniConfGenList *_l) : l(_l), i(*_l) { }
    virtual UniConfListGen::~UniConfListGen() { delete l; }

    UniConfGenList *l;
    UniConfGenList::Iter i;

    /***** Overridden members *****/

    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth); 
    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual bool isok();
    virtual Iter *iterator(const UniConfKey &key);
};


#endif // __UNICONFLISTGEN_H
