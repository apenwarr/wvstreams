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
#include "wvlinkerhack.h"

WV_LINK(UniSubtreeGen);


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


bool UniSubtreeGen::keymap(const UniConfKey &unmapped_key, UniConfKey &mapped_key)
{
    if (unmapped_key.isempty())
        mapped_key = subkey;
    else
        mapped_key = UniConfKey(subkey, unmapped_key);
    return true;
}

bool UniSubtreeGen::reversekeymap(const UniConfKey &mapped_key, UniConfKey &unmapped_key)
{
    UniConfKey _unmapped_key;
    bool result = subkey.suborsame(mapped_key, _unmapped_key);
    if (result)
        unmapped_key = _unmapped_key;
    return result;
}
