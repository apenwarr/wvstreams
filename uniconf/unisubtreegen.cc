/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen for returning only a particular subtree of a given generator.
 */
#include "unisubtreegen.h"
#include "wvbuf.h"
#include "wvtclstring.h"
#include "wvmoniker.h"

static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    WvConstInPlaceBuf buf(s, s.len());
    WvString one(wvtcl_getword(buf)), two(wvtcl_getword(buf));

    if (!one) one = "";
    if (!two) two = "";
    
    if (obj)
	return new UniSubtreeGen(mutate<IUniConfGen>(obj), one);
    else
    {
	return new UniSubtreeGen(wvcreate<IUniConfGen>(one), two);
    }
}

static WvMoniker<IUniConfGen> subtreereg("subtree", creator);


UniSubtreeGen::UniSubtreeGen(IUniConfGen *gen, const UniConfKey &_subkey)
    : UniFilterGen(gen), subkey(_subkey)
{
    // nothing special
}


UniConfKey UniSubtreeGen::keymap(const UniConfKey &key)
{
    if (key != "")
        return UniConfKey(subkey, key);
    else
        return subkey;
}

UniConfKey UniSubtreeGen::reversekeymap(const UniConfKey &key)
{
    if (key.numsegments() >= subkey.numsegments())
        return UniConfKey(key.removefirst(subkey.numsegments()));

    return key;
}
